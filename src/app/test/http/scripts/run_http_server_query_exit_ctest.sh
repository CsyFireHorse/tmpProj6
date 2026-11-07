#!/usr/bin/env sh
set -e
if [ "$#" -lt 3 ]; then
  echo "Usage: run_http_server_query_exit_ctest.sh <motapp_demo_exe> <fixture_bin_dir> <repo_root>"
  exit 1
fi

MOTAPP="$1"
FIXTURE_BIN="$2"
REPO_ROOT="$3"
# Keep paths usable by shell tools and native Windows executables. GPT-6.
if command -v cygpath >/dev/null 2>&1; then
  MOTAPP=$(cygpath -m "$MOTAPP")
  FIXTURE_BIN=$(cygpath -m "$FIXTURE_BIN")
  REPO_ROOT=$(cygpath -m "$REPO_ROOT")
fi
# HTTP routes are protocol arguments, not native filesystem paths.
export MSYS2_ARG_CONV_EXCL='/zmot/api/'
# Use a dedicated port so unrelated local services cannot occupy this test.
HTTP_PORT=28768
SESSION="" # AI modification signature: GPT-6.
CASE_DIR="$FIXTURE_BIN/case_http_server_query_exit"
SERVER_OUT="$CASE_DIR/server_stdout.log"
SERVER_ERR="$CASE_DIR/server_stderr.log"
QUERY_OUT="$CASE_DIR/query_stdout.log"
EXIT_OUT="$CASE_DIR/exit_stdout.log"

if [ ! -x "$MOTAPP" ] && [ ! -f "$MOTAPP" ]; then
  echo "motapp demo not found: $MOTAPP"
  exit 1
fi
if [ ! -d "$REPO_ROOT" ]; then
  echo "repo root not found: $REPO_ROOT"
  exit 1
fi

mkdir -p "$CASE_DIR"
rm -f "$SERVER_OUT" "$SERVER_ERR" "$QUERY_OUT" "$EXIT_OUT"

cleanup() {
  if [ "${SERVER_PID:-}" != "" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
    "$MOTAPP" http_client 127.0.0.1 "$HTTP_PORT" /zmot/api/server_exit POST --session "$SESSION" > /dev/null 2>&1 || true
    sleep 1
  fi
  if [ "${SERVER_PID:-}" != "" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
    kill "$SERVER_PID" 2>/dev/null || true
  fi
  if [ "${SERVER_PID:-}" != "" ]; then
    wait "$SERVER_PID" 2>/dev/null || true
  fi
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM

# Track the server itself so cleanup and wait use the same process.
(cd "$REPO_ROOT" && exec "$MOTAPP" axis_ini --http "--port=$HTTP_PORT" > "$SERVER_OUT" 2> "$SERVER_ERR") &
SERVER_PID=$!

READY=0
TRIES=0
while [ "$TRIES" -lt 80 ]; do
  "$MOTAPP" http_client 127.0.0.1 "$HTTP_PORT" /zmot/api/open POST '{}' > "$QUERY_OUT" 2>&1 || true
  SESSION=$(sed -n 's/.*"session" *: *"\([0-9][0-9]*\)".*/\1/p' "$QUERY_OUT")
  if grep -q "Status: 200" "$QUERY_OUT" && [ -n "$SESSION" ]; then
    READY=1
    break
  fi
  if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "http server exited before ready"
    cat "$SERVER_OUT" 2>/dev/null || true
    cat "$SERVER_ERR" 2>/dev/null || true
    exit 1
  fi
  TRIES=$((TRIES + 1))
  sleep 0.1
done

if [ "$READY" != "1" ]; then
  echo "http server did not become ready"
  cat "$QUERY_OUT" 2>/dev/null || true
  cat "$SERVER_OUT" 2>/dev/null || true
  cat "$SERVER_ERR" 2>/dev/null || true
  exit 1
fi

"$MOTAPP" http_client 127.0.0.1 "$HTTP_PORT" /zmot/api/status_latest GET --session "$SESSION" > "$QUERY_OUT" 2>&1 || true
grep -q "Status: 200" "$QUERY_OUT" || {
  echo "status query did not return HTTP 200"
  cat "$QUERY_OUT"
  exit 1
}
grep -q '"axis"' "$QUERY_OUT" || {
  echo "status query response does not contain axis JSON"
  cat "$QUERY_OUT"
  exit 1
}

"$MOTAPP" http_client 127.0.0.1 "$HTTP_PORT" /zmot/api/server_exit POST --session "$SESSION" > "$EXIT_OUT" 2>&1 || true
grep -q "Status: 200" "$EXIT_OUT" || {
  echo "server_exit did not return HTTP 200"
  cat "$EXIT_OUT"
  exit 1
}
grep -Eq '"code":[[:space:]]*0' "$EXIT_OUT" || grep -Eq '"message":[[:space:]]*"ok"' "$EXIT_OUT" || {
  echo "server_exit response is not JSON ok envelope"
  cat "$EXIT_OUT"
  exit 1
}

TRIES=0
while [ "$TRIES" -lt 100 ]; do
  if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    wait "$SERVER_PID"
    trap - EXIT
    exit 0
  fi
  TRIES=$((TRIES + 1))
  sleep 0.1
done

echo "http server did not exit after server_exit"
cat "$SERVER_OUT" 2>/dev/null || true
cat "$SERVER_ERR" 2>/dev/null || true
exit 1
