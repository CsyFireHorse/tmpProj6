#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Find or remove Chinese comments in C/C++ sources.

Comments are detected with a small C++ lexer, so comment markers inside
string literals, character literals and raw string literals are ignored.

Examples:
    python tools/filter_cn_comments.py
    python tools/filter_cn_comments.py src --ext cpp,h,hpp
    python tools/filter_cn_comments.py src -w --backup
    python tools/filter_cn_comments.py src -w --dry-run
    python tools/filter_cn_comments.py src -w --mode strip
    python tools/filter_cn_comments.py src --check --json

Modes:
    remove (default)  drop the whole comment (line count is preserved)
    strip             drop only the Chinese characters, keep the comment

Exit status:
    0  success
    1  --check was given and Chinese comments remain
    2  invalid arguments, or no files were scanned
"""

from __future__ import print_function

import argparse
import fnmatch
import json
import os
import shutil
import sys
import tempfile
from collections import namedtuple

DEFAULT_EXTS = ("cpp", "cc", "cxx", "c++", "h", "hpp", "hxx", "hh", "h++", "inl")

DEFAULT_EXCLUDE_DIRS = (
    ".git",
    ".hg",
    ".svn",
    "__pycache__",
    ".venv",
    "venv",
    "node_modules",
    "build",
)

BOMS = (
    (b"\x00\x00\xfe\xff", "utf-32-be"),
    (b"\xff\xfe\x00\x00", "utf-32-le"),
    (b"\xfe\xff", "utf-16-be"),
    (b"\xff\xfe", "utf-16-le"),
    (b"\xef\xbb\xbf", "utf-8"),
)

CJK_RANGES = (
    (0x2E80, 0x2EFF),    # CJK radicals supplement
    (0x3000, 0x303F),    # CJK symbols and punctuation
    (0x3400, 0x4DBF),    # CJK unified ideographs extension A
    (0x4E00, 0x9FFF),    # CJK unified ideographs
    (0xF900, 0xFAFF),    # CJK compatibility ideographs
    (0xFE30, 0xFE4F),    # CJK compatibility forms
    (0xFF00, 0xFFEF),    # halfwidth and fullwidth forms
    (0x20000, 0x2FA1F),  # CJK unified ideographs extension B-F
)

RAW_PREFIXES = ('u8R"', 'uR"', 'UR"', 'LR"', 'R"')

MAX_PREVIEW = 160

Comment = namedtuple("Comment", "kind start end line column text")


def _split_values(values):
    result = []
    for value in values:
        for part in value.split(","):
            part = part.strip()
            if part:
                result.append(part)
    return result


def _ext_set(values):
    result = set()
    for ext in _split_values(values):
        ext = ext.lower()
        if not ext.startswith("."):
            ext = "." + ext
        result.add(ext)
    return result


def _is_excluded(name, patterns):
    lower = name.lower()
    return any(fnmatch.fnmatch(lower, pattern.lower()) for pattern in patterns)


def _walk_files(paths, allowed_exts, exclude_patterns, include_hidden):
    seen = set()
    for start in paths:
        if os.path.isfile(start):
            full = os.path.abspath(start)
            if full not in seen:
                seen.add(full)
                yield start
            continue

        if not os.path.isdir(start):
            print("warning: path not found: {}".format(start), file=sys.stderr)
            continue

        for root, dirs, files in os.walk(start):
            dirs[:] = [
                d
                for d in dirs
                if not _is_excluded(d, exclude_patterns)
                and (include_hidden or not d.startswith("."))
            ]

            for name in sorted(files):
                if not include_hidden and name.startswith("."):
                    continue
                if allowed_exts and os.path.splitext(name)[1].lower() not in allowed_exts:
                    continue
                full = os.path.abspath(os.path.join(root, name))
                if full in seen:
                    continue
                seen.add(full)
                yield os.path.join(root, name)


def _is_cjk(char):
    codepoint = ord(char)
    for low, high in CJK_RANGES:
        if low <= codepoint <= high:
            return True
    return False


def _has_cjk(text):
    for char in text:
        if _is_cjk(char):
            return True
    return False


def _is_spliced(text, newline):
    index = newline - 1
    if index >= 0 and text[index] == "\r":
        index -= 1
    return index >= 0 and text[index] == "\\"


def _skip_raw_string(text, index):
    for prefix in RAW_PREFIXES:
        if not text.startswith(prefix, index):
            continue
        if index > 0 and (text[index - 1].isalnum() or text[index - 1] == "_"):
            continue
        quote = index + len(prefix) - 1
        paren = text.find("(", quote + 1)
        if paren < 0:
            return None
        delimiter = text[quote + 1:paren]
        if len(delimiter) > 16:
            return None
        if any(ch in ' ()\\\t\r\n' for ch in delimiter):
            return None
        terminator = ")" + delimiter + '"'
        end = text.find(terminator, paren + 1)
        if end < 0:
            return len(text)
        return end + len(terminator)
    return None


def _skip_string(text, index):
    end = index + 1
    length = len(text)
    while end < length:
        char = text[end]
        if char == "\\":
            end += 2
            continue
        if char == '"':
            return end + 1
        end += 1
    return length


def _skip_char(text, index):
    end = index + 1
    length = len(text)
    while end < length:
        char = text[end]
        if char == "\\":
            end += 2
            continue
        if char == "'":
            return end + 1
        end += 1
    return length


def _advance(text, start, end, line, column):
    for index in range(start, end):
        if text[index] == "\n":
            line += 1
            column = 1
        else:
            column += 1
    return end, line, column


def iter_comments(text):
    length = len(text)
    index = 0
    line = 1
    column = 1
    while index < length:
        char = text[index]

        if char == "\n":
            index += 1
            line += 1
            column = 1
            continue

        raw_end = _skip_raw_string(text, index)
        if raw_end is not None:
            index, line, column = _advance(text, index, raw_end, line, column)
            continue

        if char == '"':
            end = _skip_string(text, index)
            index, line, column = _advance(text, index, end, line, column)
            continue

        if char == "'":
            end = _skip_char(text, index)
            index, line, column = _advance(text, index, end, line, column)
            continue

        if char == "/" and index + 1 < length and text[index + 1] == "/":
            start = index
            start_line = line
            start_column = column
            end = index + 2
            while end < length:
                if text[end] == "\n":
                    if _is_spliced(text, end):
                        end += 1
                        continue
                    break
                end += 1
            yield Comment(
                "line", start, end, start_line, start_column, text[start:end]
            )
            index, line, column = _advance(text, index, end, line, column)
            continue

        if char == "/" and index + 1 < length and text[index + 1] == "*":
            start = index
            start_line = line
            start_column = column
            close = text.find("*/", index + 2)
            end = length if close < 0 else close + 2
            yield Comment(
                "block", start, end, start_line, start_column, text[start:end]
            )
            index, line, column = _advance(text, index, end, line, column)
            continue

        index += 1
        column += 1


def _find_chinese_comments(text):
    return [comment for comment in iter_comments(text) if _has_cjk(comment.text)]


def _preview(comment):
    text = comment.text.replace("\r", " ").replace("\n", " ")
    text = " ".join(text.split())
    if len(text) > MAX_PREVIEW:
        text = text[:MAX_PREVIEW] + "..."
    return text


def _read_source(path, fallback_encoding):
    with open(path, "rb") as handle:
        data = handle.read()

    for bom, bom_encoding in BOMS:
        if data.startswith(bom):
            try:
                text = data[len(bom):].decode(bom_encoding)
            except UnicodeDecodeError as exc:
                return (
                    data[len(bom):].decode(bom_encoding, "replace"),
                    bom_encoding,
                    bom,
                    exc,
                )
            return text, bom_encoding, bom, None

    try:
        return data.decode(fallback_encoding), fallback_encoding, b"", None
    except UnicodeDecodeError as exc:
        return (
            data.decode(fallback_encoding, "replace"),
            fallback_encoding,
            b"",
            exc,
        )


def _blank_comment(comment):
    if comment.kind == "block" and "\n" not in comment.text and "\r" not in comment.text:
        return " "
    return "".join(ch for ch in comment.text if ch in "\r\n")


def _apply(text, comments, mode):
    parts = []
    last = 0
    for comment in comments:
        parts.append(text[last:comment.start])
        if mode == "strip":
            parts.append("".join(ch for ch in comment.text if not _is_cjk(ch)))
        else:
            parts.append(_blank_comment(comment))
        last = comment.end
    parts.append(text[last:])
    return "".join(parts)


def _write_source(path, text, encoding, bom, backup):
    if backup:
        shutil.copy2(path, path + ".bak")

    data = bom + text.encode(encoding)
    directory = os.path.dirname(os.path.abspath(path))
    handle, temp_path = tempfile.mkstemp(dir=directory, prefix=".cnc_", suffix=".tmp")
    try:
        with os.fdopen(handle, "wb") as stream:
            stream.write(data)
        shutil.copymode(path, temp_path)
        os.replace(temp_path, path)
    except Exception:
        try:
            os.remove(temp_path)
        except OSError:
            pass
        raise


def _scan_file(path, args):
    text, encoding, bom, decode_error = _read_source(path, args.encoding)
    comments = _find_chinese_comments(text)

    result = {
        "path": path,
        "encoding": encoding,
        "decode_error": None if decode_error is None else str(decode_error),
        "findings": [
            {
                "kind": comment.kind,
                "line": comment.line,
                "column": comment.column,
                "text": _preview(comment),
            }
            for comment in comments
        ],
        "written": False,
        "would_write": False,
    }

    if args.write and comments and decode_error is None:
        if args.dry_run:
            result["would_write"] = True
        else:
            new_text = _apply(text, comments, args.mode)
            _write_source(path, new_text, encoding, bom, args.backup)
            result["written"] = True

    return result


def _print_human(results, args):
    total = 0
    changed = 0
    for result in results:
        findings = result["findings"]
        if not findings and not result["decode_error"]:
            continue

        print(result["path"])
        if result["decode_error"] and args.level != "file":
            print("  decode error: {}".format(result["decode_error"]))

        if args.level != "file":
            shown = findings
            if args.max_per_file > 0:
                shown = findings[: args.max_per_file]
            for item in shown:
                if args.level == "detail":
                    print(
                        "  line {:<5} col {:<4} {:<5} {}".format(
                            item["line"], item["column"], item["kind"], item["text"]
                        )
                    )
                else:
                    print(
                        "  {}:{}: {}".format(item["line"], item["column"], item["text"])
                    )
            if len(shown) < len(findings):
                print(
                    "  ... {} more comment(s) not shown".format(
                        len(findings) - len(shown)
                    )
                )

        if result["written"]:
            print("  removed Chinese comment(s)")
            changed += 1
        elif result["would_write"]:
            print("  would remove Chinese comment(s) (dry run)")
        total += len(findings)
        print("")

    files_with_findings = sum(1 for result in results if result["findings"])

    if files_with_findings:
        action = "Found"
        if args.write:
            action = "Updated" if not args.dry_run else "Would update"
        print(
            "{} {} Chinese comment(s) in {} file(s) ({} scanned).".format(
                action, total, files_with_findings, len(results)
            )
        )
        if args.write and not args.dry_run:
            print("Rewrote {} file(s).".format(changed))
    else:
        print(
            "No Chinese comments found in {} file(s).".format(len(results))
        )


def _print_json(results):
    payload = {
        "files": [result for result in results if result["findings"] or result["decode_error"]],
        "summary": {
            "scanned": len(results),
            "files_with_findings": sum(1 for result in results if result["findings"]),
            "comments": sum(len(result["findings"]) for result in results),
            "written": sum(1 for result in results if result["written"]),
            "would_write": sum(1 for result in results if result["would_write"]),
        },
    }
    print(json.dumps(payload, ensure_ascii=False, indent=2))


def main(argv=None):
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except AttributeError:
        pass

    parser = argparse.ArgumentParser(
        description="Find or remove Chinese comments in C/C++ sources.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "Examples:\n"
            "  python tools/filter_cn_comments.py\n"
            "  python tools/filter_cn_comments.py src --ext cpp,h,hpp\n"
            "  python tools/filter_cn_comments.py src -w --backup\n"
            "  python tools/filter_cn_comments.py src -w --mode strip\n"
        ),
    )
    parser.add_argument(
        "paths",
        nargs="*",
        help="files or directories to scan (default: current directory)",
    )
    parser.add_argument(
        "-e",
        "--ext",
        action="append",
        default=[],
        help="comma separated extensions (default: cpp,cc,cxx,h,hpp,hxx,...)",
    )
    parser.add_argument(
        "-x",
        "--exclude",
        action="append",
        default=[],
        help="extra directory names/globs to skip (repeatable)",
    )
    parser.add_argument(
        "--encoding",
        default="utf-8",
        help="fallback encoding when no BOM is present (default: utf-8)",
    )
    parser.add_argument(
        "--include-hidden",
        action="store_true",
        help="also scan hidden files and directories",
    )
    parser.add_argument(
        "--mode",
        choices=("remove", "strip"),
        default="remove",
        help="remove whole comments or only Chinese characters (default: remove)",
    )
    parser.add_argument(
        "-w",
        "--write",
        action="store_true",
        help="rewrite files in place while filtering comments",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="with --write, show what would change without writing",
    )
    parser.add_argument(
        "--backup",
        action="store_true",
        help="with --write, keep a .bak copy of each modified file",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="exit with status 1 when Chinese comments remain",
    )
    parser.add_argument(
        "--max-per-file",
        type=int,
        default=50,
        help="maximum comments to print per file, 0 = unlimited (default: 50)",
    )
    parser.add_argument(
        "-l",
        "--level",
        choices=("file", "line", "detail"),
        default="line",
        help="output level: file, line, or detail (default: line)",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="print machine-readable JSON instead of text",
    )
    args = parser.parse_args(argv)

    if args.dry_run and not args.write:
        print("error: --dry-run requires --write", file=sys.stderr)
        return 2
    if args.backup and not args.write:
        print("error: --backup requires --write", file=sys.stderr)
        return 2

    paths = args.paths or ["."]
    allowed_exts = _ext_set(args.ext or DEFAULT_EXTS)
    exclude_patterns = list(DEFAULT_EXCLUDE_DIRS) + _split_values(args.exclude)

    results = []
    scanned = 0

    for path in _walk_files(paths, allowed_exts, exclude_patterns, args.include_hidden):
        scanned += 1
        try:
            results.append(_scan_file(path, args))
        except OSError as exc:
            print("error: cannot process {}: {}".format(path, exc), file=sys.stderr)

    if scanned == 0:
        print("error: no files scanned", file=sys.stderr)
        return 2

    if args.json:
        _print_json(results)
    else:
        _print_human(results, args)

    if args.check:
        remaining = sum(len(result["findings"]) for result in results if not result["written"])
        if remaining:
            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
