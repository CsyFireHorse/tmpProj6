#include "memory/MemoryOperator.hpp"

#include "common/thirdlib/nlohmann/json.hpp"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdint.h>

namespace {

std::string trim(const std::string& s) {
    std::size_t first = 0;
    while (first < s.size() && std::isspace(static_cast<unsigned char>(s[first]))) {
        ++first;
    }

    std::size_t last = s.size();
    while (last > first && std::isspace(static_cast<unsigned char>(s[last - 1]))) {
        --last;
    }

    return s.substr(first, last - first);
}

std::size_t alignUp(std::size_t value, std::size_t align) {
    if (align <= 1) {
        return value;
    }
    const std::size_t rem = value % align;
    return rem == 0 ? value : value + (align - rem);
}

void skipSpaces(const std::string& text, std::size_t& pos) {
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
        ++pos;
    }
}

bool readIdent(const std::string& text, std::size_t& pos, std::string& out) {
    skipSpaces(text, pos);
    if (pos >= text.size() ||
        !(std::isalpha(static_cast<unsigned char>(text[pos])) || text[pos] == '_')) {
        return false;
    }

    const std::size_t begin = pos++;
    while (pos < text.size() &&
           (std::isalnum(static_cast<unsigned char>(text[pos])) || text[pos] == '_')) {
        ++pos;
    }

    out = text.substr(begin, pos - begin);
    return true;
}

bool consumeChar(const std::string& text, std::size_t& pos, char ch) {
    skipSpaces(text, pos);
    if (pos >= text.size() || text[pos] != ch) {
        return false;
    }
    ++pos;
    return true;
}

bool consumeWord(const std::string& text, std::size_t& pos, const char* word) {
    skipSpaces(text, pos);
    const std::size_t len = std::strlen(word);
    if (text.compare(pos, len, word) != 0) {
        return false;
    }

    const std::size_t end = pos + len;
    if (end < text.size() &&
        (std::isalnum(static_cast<unsigned char>(text[end])) || text[end] == '_')) {
        return false;
    }

    pos = end;
    return true;
}

bool parseSize(const nlohmann::json& obj, const char* key, std::size_t& value) {
    if (!obj.count(key) || !obj[key].is_number_unsigned()) {
        return false;
    }
    value = static_cast<std::size_t>(obj[key].get<unsigned long long>());
    return true;
}

template <typename T>
void toBytes(const T& value, std::vector<unsigned char>& out) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(&value);
    out.assign(p, p + sizeof(T));
}

template <typename T>
bool parseInteger(const std::string& text, T& value) {
    char* end = NULL;
    errno = 0;
    const long long v = std::strtoll(text.c_str(), &end, 10);
    if (errno != 0 || end == text.c_str() || *end != '\0') {
        return false;
    }
    if (v < static_cast<long long>(std::numeric_limits<T>::min()) ||
        v > static_cast<long long>(std::numeric_limits<T>::max())) {
        return false;
    }
    value = static_cast<T>(v);
    return true;
}

}  // namespace

MemoryOperator::MemoryOperator()
    : base_(NULL), bytes_(0), rootSize_(0), maxAlign_(1) {}

const MemoryRootDecl* MemoryOperator::declAt(std::size_t index) const {
    if (index >= decls_.size()) {
        return NULL;
    }
    return &decls_[index];
}

int MemoryOperator::loadTypeLayoutFile(const std::string& path) {
    std::ifstream in(path.c_str());
    if (!in) {
        return MEMOP_ERR_IO;
    }

    nlohmann::json root;
    try {
        in >> root;
    } catch (...) {
        return MEMOP_ERR_JSON;
    }

    return loadTypeLayoutJson(root);
}

int MemoryOperator::loadTypeLayoutContent(const std::string& content) {
    nlohmann::json root;
    try {
        root = nlohmann::json::parse(content);
    } catch (...) {
        return MEMOP_ERR_JSON;
    }

    return loadTypeLayoutJson(root);
}

int MemoryOperator::loadTypeLayoutJson(const nlohmann::json& root) {
    if (!root.count("types") || !root["types"].is_object()) {
        return MEMOP_ERR_LAYOUT;
    }

    types_.clear();
    for (nlohmann::json::const_iterator it = root["types"].begin(); it != root["types"].end();
         ++it) {
        if (!it.value().is_object()) {
            return MEMOP_ERR_LAYOUT;
        }

        TypeInfo type;
        if (!parseSize(it.value(), "size", type.size) ||
            !parseSize(it.value(), "align", type.align) || type.size == 0 || type.align == 0) {
            return MEMOP_ERR_LAYOUT;
        }
        type.kind = it.value().value("kind", "pod");

        if (it.value().count("fields")) {
            if (!it.value()["fields"].is_object()) {
                return MEMOP_ERR_LAYOUT;
            }

            for (nlohmann::json::const_iterator fit = it.value()["fields"].begin();
                 fit != it.value()["fields"].end(); ++fit) {
                FieldInfo field;
                if (!fit.value().is_object() || !fit.value().count("type") ||
                    !fit.value()["type"].is_string() ||
                    !parseSize(fit.value(), "offset", field.offset) ||
                    !parseSize(fit.value(), "size", field.size)) {
                    return MEMOP_ERR_LAYOUT;
                }

                field.type = fit.value()["type"].get<std::string>();
                type.fields.push_back(std::make_pair(fit.key(), field));
            }
        }

        types_.push_back(std::make_pair(it.key(), type));
    }

    for (std::size_t i = 0; i < types_.size(); ++i) {
        const TypeInfo& owner = types_[i].second;
        for (std::size_t j = 0; j < owner.fields.size(); ++j) {
            const FieldInfo& field = owner.fields[j].second;
            const TypeInfo* fieldType = findType(field.type);
            if (!fieldType || field.size != fieldType->size || field.offset > owner.size ||
                field.size > owner.size - field.offset) {
                return MEMOP_ERR_LAYOUT;
            }
        }
    }

    return MEMOP_OK;
}

int MemoryOperator::configure(const std::string& initConfig) {
    std::vector<Decl> decls;
    int rc = parseInitConfig(initConfig, decls);
    if (rc != MEMOP_OK) {
        return rc;
    }

    symbols_.clear();
    decls_.clear();
    rootSize_ = 0;
    maxAlign_ = 1;

    for (std::size_t i = 0; i < decls.size(); ++i) {
        const TypeInfo* type = findType(decls[i].type);
        if (!type || findSymbol(decls[i].name)) {
            return MEMOP_ERR_SYMBOL;
        }

        rootSize_ = alignUp(rootSize_, type->align);
        MemoryRootDecl root;
        root.type = decls[i].type;
        root.name = decls[i].name;
        root.offset = rootSize_;
        root.size = type->size;
        decls_.push_back(root);
        addSymbolTree(decls[i].name, decls[i].type, rootSize_);
        rootSize_ += type->size;
        maxAlign_ = std::max(maxAlign_, type->align);
    }

    rootSize_ = alignUp(rootSize_, maxAlign_);
    return MEMOP_OK;
}

int MemoryOperator::compile(const std::string& script, std::vector<AssignOp>& ops) const {
    std::vector<AssignOp> result;
    std::size_t begin = 0;

    for (std::size_t pos = 0; pos <= script.size(); ++pos) {
        const bool atEnd = pos == script.size();
        if (!atEnd && script[pos] != ';' && script[pos] != '\n' && script[pos] != '\r') {
            continue;
        }

        const std::string line = trim(script.substr(begin, pos - begin));
        if (!atEnd && script[pos] == '\r' && pos + 1 < script.size() && script[pos + 1] == '\n') {
            ++pos;
        }
        begin = pos + 1;

        if (line.empty()) {
            continue;
        }

        const std::size_t eq = line.find('=');
        if (eq == std::string::npos || line.find('=', eq + 1) != std::string::npos) {
            return MEMOP_ERR_ASSIGN;
        }

        const std::string lhs = trim(line.substr(0, eq));
        const std::string rhs = trim(line.substr(eq + 1));
        const SymbolInfo* dst = findSymbol(lhs);
        if (!dst) {
            return MEMOP_ERR_SYMBOL;
        }

        AssignOp op;
        op.dstOffset = dst->offset;
        op.len = dst->size;
        op.srcOffset = 0;

        if (const SymbolInfo* src = findSymbol(rhs)) {
            if (src->type != dst->type || src->size != dst->size) {
                return MEMOP_ERR_ASSIGN;
            }
            op.kind = AssignOp::CopyBytes;
            op.srcOffset = src->offset;
        } else {
            op.kind = AssignOp::WriteValue;
            int rc = encodeLiteral(rhs, *dst, op.value);
            if (rc != MEMOP_OK || op.value.size() != op.len) {
                return rc == MEMOP_OK ? MEMOP_ERR_ASSIGN : rc;
            }
        }

        result.push_back(op);
    }

    ops.swap(result);
    return MEMOP_OK;
}

int MemoryOperator::execute(const std::string& script) {
    std::vector<AssignOp> ops;
    int rc = compile(script, ops);
    if (rc != MEMOP_OK) {
        return rc;
    }

    for (std::size_t i = 0; i < ops.size(); ++i) {
        if (ops[i].kind == AssignOp::CopyBytes) {
            rc = copy(ops[i].dstOffset, ops[i].srcOffset, ops[i].len);
        } else {
            rc = assign(ops[i].dstOffset, &ops[i].value[0], ops[i].len);
        }
        if (rc != MEMOP_OK) {
            return rc;
        }
    }

    return MEMOP_OK;
}

void MemoryOperator::bindMemory(void* base, std::size_t bytes) {
    base_ = base;
    bytes_ = bytes;
}

int MemoryOperator::assign(std::size_t offset, const void* value, std::size_t len) {
    if (!base_ || !value) {
        return MEMOP_ERR_ASSIGN;
    }
    if (offset > bytes_ || len > bytes_ - offset) {
        return MEMOP_ERR_RANGE;
    }

    std::memcpy(static_cast<unsigned char*>(base_) + offset, value, len);
    return MEMOP_OK;
}

int MemoryOperator::copy(std::size_t offset, std::size_t srcOffset, std::size_t len) {
    if (!base_) {
        return MEMOP_ERR_ASSIGN;
    }
    if (offset > bytes_ || len > bytes_ - offset || srcOffset > bytes_ ||
        len > bytes_ - srcOffset) {
        return MEMOP_ERR_RANGE;
    }

    unsigned char* p = static_cast<unsigned char*>(base_);
    std::memmove(p + offset, p + srcOffset, len);
    return MEMOP_OK;
}

int MemoryOperator::read(std::size_t offset, void* dst, std::size_t len) const {
    if (!base_ || !dst) {
        return MEMOP_ERR_ASSIGN;
    }
    if (offset > bytes_ || len > bytes_ - offset) {
        return MEMOP_ERR_RANGE;
    }

    std::memcpy(dst, static_cast<const unsigned char*>(base_) + offset, len);
    return MEMOP_OK;
}

int MemoryOperator::compare(std::size_t offset, const void* value, std::size_t len) const {
    return compare(offset, value, len, 0);
}

namespace {

template <typename T>
bool memory_op_eval_ordered(T actual, T expected, int cmp) {
    switch (cmp) {
        case 2:
            return actual < expected;
        case 3:
            return actual <= expected;
        case 4:
            return actual > expected;
        case 5:
            return actual >= expected;
        default:
            return false;
    }
}

template <typename T>
int memory_op_compare_ordered(const unsigned char* actual, const unsigned char* expected,
                              std::size_t len, int cmp) {
    if (len != sizeof(T)) {
        return MEMOP_ERR_ASSIGN;
    }
    T actualValue;
    T expectedValue;
    std::memcpy(&actualValue, actual, sizeof(T));
    std::memcpy(&expectedValue, expected, sizeof(T));
    return memory_op_eval_ordered(actualValue, expectedValue, cmp) ? MEMOP_OK : MEMOP_ERR_EXPECT;
}

int memory_op_compare_by_kind(const std::string& kind, const unsigned char* actual,
                              const unsigned char* expected, std::size_t len, int cmp) {
    if (kind == "bool") {
        return memory_op_compare_ordered<unsigned char>(actual, expected, len, cmp);
    }
    if (kind == "int32") {
        return memory_op_compare_ordered<int32_t>(actual, expected, len, cmp);
    }
    if (kind == "uint32") {
        return memory_op_compare_ordered<uint32_t>(actual, expected, len, cmp);
    }
    if (kind == "uint16") {
        return memory_op_compare_ordered<uint16_t>(actual, expected, len, cmp);
    }
    if (kind == "float") {
        return memory_op_compare_ordered<float>(actual, expected, len, cmp);
    }
    if (kind == "double") {
        return memory_op_compare_ordered<double>(actual, expected, len, cmp);
    }
    return MEMOP_ERR_ASSIGN;
}

}  // namespace

bool MemoryOperator::symbolAllowsOrderedCompare(const SymbolInfo& sym) const {
    const TypeInfo* type = findType(sym.type);
    if (!type) {
        return false;
    }
    return type->kind == "bool" || type->kind == "int32" || type->kind == "uint32" ||
           type->kind == "uint16" || type->kind == "float" || type->kind == "double";
}

const SymbolInfo* MemoryOperator::findSymbolByOffset(std::size_t offset, std::size_t len) const {
    const SymbolInfo* match = NULL;
    std::size_t matchNameLen = 0;
    for (std::size_t i = 0; i < symbols_.size(); ++i) {
        const SymbolInfo& sym = symbols_[i].second;
        if (sym.offset == offset && sym.size == len) {
            const std::size_t nameLen = symbols_[i].first.size();
            if (match == NULL || nameLen > matchNameLen) {
                match = &sym;
                matchNameLen = nameLen;
            }
        }
    }
    return match;
}

int MemoryOperator::compare(std::size_t offset, const void* value, std::size_t len,
                            int cmp) const {
    if (!value) {
        return MEMOP_ERR_ASSIGN;
    }
    if (cmp < 0 || cmp > 5) {
        return MEMOP_ERR_ASSIGN;
    }
    unsigned char actual[8];
    if (len > sizeof(actual)) {
        return MEMOP_ERR_ASSIGN;
    }
    const int rc = read(offset, actual, len);
    if (rc != MEMOP_OK) {
        return rc;
    }

    const unsigned char* expected = static_cast<const unsigned char*>(value);
    if (cmp == 0 || cmp == 1) {
        const bool equal = std::memcmp(actual, expected, len) == 0;
        if (cmp == 0) {
            return equal ? MEMOP_OK : MEMOP_ERR_EXPECT;
        }
        return equal ? MEMOP_ERR_EXPECT : MEMOP_OK;
    }

    const SymbolInfo* sym = findSymbolByOffset(offset, len);
    if (!sym) {
        return MEMOP_ERR_ASSIGN;
    }
    const TypeInfo* type = findType(sym->type);
    if (!type || !symbolAllowsOrderedCompare(*sym)) {
        return MEMOP_ERR_ASSIGN;
    }
    return memory_op_compare_by_kind(type->kind, actual, expected, len, cmp);
}

const SymbolInfo* MemoryOperator::findSymbol(const std::string& name) const {
    for (std::size_t i = 0; i < symbols_.size(); ++i) {
        if (symbols_[i].first == name) {
            return &symbols_[i].second;
        }
    }
    return NULL;
}

int MemoryOperator::parseInitConfig(const std::string& text, std::vector<Decl>& decls) const {
    std::size_t pos = 0;
    if (!consumeChar(text, pos, '{')) {
        return MEMOP_ERR_INIT;
    }

    for (;;) {
        skipSpaces(text, pos);
        if (pos < text.size() && text[pos] == '}') {
            ++pos;
            break;
        }

        Decl decl;
        if (!readIdent(text, pos, decl.type) || !readIdent(text, pos, decl.name)) {
            return MEMOP_ERR_INIT;
        }

        skipSpaces(text, pos);
        if (pos < text.size() && text[pos] == '=') {
            ++pos;
            if (!consumeWord(text, pos, "nullptr")) {
                return MEMOP_ERR_INIT;
            }
        }

        if (!consumeChar(text, pos, ';')) {
            return MEMOP_ERR_INIT;
        }

        decls.push_back(decl);
    }

    consumeChar(text, pos, ';');
    skipSpaces(text, pos);
    return pos == text.size() ? MEMOP_OK : MEMOP_ERR_INIT;
}

int MemoryOperator::encodeLiteral(const std::string& text, const SymbolInfo& dst,
                                  std::vector<unsigned char>& bytes) const {
    const TypeInfo* type = findType(dst.type);
    if (!type) {
        return MEMOP_ERR_SYMBOL;
    }

    if (type->kind == "bool") {
        if (text == "true") {
            const bool v = true;
            toBytes(v, bytes);
            return MEMOP_OK;
        }
        if (text == "false") {
            const bool v = false;
            toBytes(v, bytes);
            return MEMOP_OK;
        }
        return MEMOP_ERR_ASSIGN;
    }

    if (type->kind == "pointer") {
        if (text != "nullptr") {
            return MEMOP_ERR_ASSIGN;
        }
        bytes.assign(dst.size, 0);
        return MEMOP_OK;
    }

    if (type->kind == "int32") {
        int32_t v = 0;
        if (!parseInteger<int32_t>(text, v)) {
            return MEMOP_ERR_ASSIGN;
        }
        toBytes(v, bytes);
        return MEMOP_OK;
    }

    if (type->kind == "uint32") {
        uint32_t v = 0;
        if (!parseInteger<uint32_t>(text, v)) {
            return MEMOP_ERR_ASSIGN;
        }
        toBytes(v, bytes);
        return MEMOP_OK;
    }

    if (type->kind == "uint16") {
        uint16_t v = 0;
        if (!parseInteger<uint16_t>(text, v)) {
            return MEMOP_ERR_ASSIGN;
        }
        toBytes(v, bytes);
        return MEMOP_OK;
    }

    if (type->kind == "float") {
        char* end = NULL;
        errno = 0;
        const float v = std::strtof(text.c_str(), &end);
        if (errno != 0 || end == text.c_str() || *end != '\0') {
            return MEMOP_ERR_ASSIGN;
        }
        toBytes(v, bytes);
        return MEMOP_OK;
    }

    if (type->kind == "double") {
        char* end = NULL;
        errno = 0;
        const double v = std::strtod(text.c_str(), &end);
        if (errno != 0 || end == text.c_str() || *end != '\0') {
            return MEMOP_ERR_ASSIGN;
        }
        toBytes(v, bytes);
        return MEMOP_OK;
    }

    return MEMOP_ERR_ASSIGN;
}

void MemoryOperator::addSymbolTree(const std::string& name, const std::string& type,
                                   std::size_t baseOffset) {
    const TypeInfo* info = findType(type);
    if (!info) {
        return;
    }

    SymbolInfo symbol;
    symbol.type = type;
    symbol.offset = baseOffset;
    symbol.size = info->size;
    symbols_.push_back(std::make_pair(name, symbol));

    for (std::size_t i = 0; i < info->fields.size(); ++i) {
        const FieldInfo& field = info->fields[i].second;
        addSymbolTree(name + "." + info->fields[i].first, field.type, baseOffset + field.offset);
    }
}

const MemoryOperator::TypeInfo* MemoryOperator::findType(const std::string& type) const {
    for (std::size_t i = 0; i < types_.size(); ++i) {
        if (types_[i].first == type) {
            return &types_[i].second;
        }
    }
    return NULL;
}
