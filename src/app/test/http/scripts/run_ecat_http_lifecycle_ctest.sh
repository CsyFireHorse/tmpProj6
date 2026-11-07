#!/usr/bin/env sh
# Date: 2026-11-05; Drafted by: GPT-6 (Codex).
set -eu

if [ "$#" -lt 3 ]; then
  echo "Usage: run_ecat_http_lifecycle_ctest.sh <motapp_demo_exe> <fixture_bin_dir> <repo_root>"
  exit 2
fi

normalize_path() {
  # Native executables and JSON payloads need Windows paths. GPT-6.
  if command -v cygpath >/dev/null 2>&1; then
    cygpath -m "$1"
  else
    printf '%s\n' "$1"
  fi
}

MOTAPP="$(normalize_path "$1")"
FIXTURE_BIN="$(normalize_path "$2")"
REPO_ROOT="$(normalize_path "$3")"
# HTTP routes are protocol arguments, not native filesystem paths.
export MSYS2_ARG_CONV_EXCL='/zmot/api/'
HTTP_PORT=18768
SESSION="" # AI modification signature: GPT-6.
CASE_DIR="$FIXTURE_BIN/case_ecat_http_lifecycle"
SERVER_OUT="$CASE_DIR/server_stdout.log"
SERVER_ERR="$CASE_DIR/server_stderr.log"
RESPONSE_OUT="$CASE_DIR/response.log"
INI_SERVER_OUT="$CASE_DIR/ini_server_stdout.log"
INI_SERVER_ERR="$CASE_DIR/ini_server_stderr.log"
INI_PATH="$REPO_ROOT/src/mhelper/data/zmc_motapp_http_io_metadata_test.ini"
ENI_PATH="$REPO_ROOT/src/mhelper/data/pse_servo1.xml"
NEXT_ENI_PATH="$REPO_ROOT/src/mhelper/data/pse_servo2.xml"

mkdir -p "$CASE_DIR"
rm -f "$SERVER_OUT" "$SERVER_ERR" "$RESPONSE_OUT" "$INI_SERVER_OUT" "$INI_SERVER_ERR"

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

request() {
  PATH_ARG="$1"
  BODY_ARG="${2:-}"
  METHOD_ARG="${3:-POST}"
  set --
  if [ -n "$SESSION" ]; then
    set -- --header "X-Zmot-Session: $SESSION"
  fi
  if [ "$METHOD_ARG" = "GET" ]; then
    curl --silent --show-error --max-time 15 --request GET \
      --write-out '\nStatus: %{http_code}\n' \
      "$@" "http://127.0.0.1:$HTTP_PORT$PATH_ARG" > "$RESPONSE_OUT" 2>&1 || true
  else
    curl --silent --show-error --max-time 15 --request POST \
      --header 'Content-Type: application/json' --data "$BODY_ARG" \
      --write-out '\nStatus: %{http_code}\n' \
      "$@" "http://127.0.0.1:$HTTP_PORT$PATH_ARG" > "$RESPONSE_OUT" 2>&1 || true
  fi
  case "$PATH_ARG" in
    /zmot/api/open|/zmot/api/open_device)
      if grep -Eq '"code": *0' "$RESPONSE_OUT"; then
        SESSION=$(sed -n 's/.*"session" *: *"\([0-9][0-9]*\)".*/\1/p' "$RESPONSE_OUT")
        [ -n "$SESSION" ] || { echo 'open did not return a session'; exit 1; }
      fi
      ;;
  esac
}

assert_contains() {
  PATTERN="$1"
  LABEL="$2"
  if ! grep -Eq "$PATTERN" "$RESPONSE_OUT"; then
    echo "$LABEL missing expected content '$PATTERN'"
    cat "$RESPONSE_OUT"
    exit 1
  fi
}

assert_status() {
  EXPECTED="$1"
  LABEL="$2"
  if ! grep -Eq "Status: *$EXPECTED" "$RESPONSE_OUT"; then
    echo "$LABEL returned an unexpected HTTP status"
    cat "$RESPONSE_OUT"
    exit 1
  fi
}

assert_zero() {
  LABEL="$1"
  assert_status 200 "$LABEL"
  if ! grep -Eq '"code": *0' "$RESPONSE_OUT"; then
    echo "$LABEL did not return code 0"
    cat "$RESPONSE_OUT"
    exit 1
  fi
}

# Track the server itself so cleanup and wait use the same process.
(cd "$REPO_ROOT" && exec "$MOTAPP" axis_ini "--port=$HTTP_PORT" > "$SERVER_OUT" 2> "$SERVER_ERR") &
SERVER_PID=$!

READY=0
TRIES=0
while [ "$TRIES" -lt 80 ]; do
  request /zmot/api/startup
  if grep -Eq 'Status: *401' "$RESPONSE_OUT"; then
    READY=1
    break
  fi
  if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "axis_ini exited before ready"
    cat "$SERVER_OUT" "$SERVER_ERR" 2>/dev/null || true
    exit 1
  fi
  TRIES=$((TRIES + 1))
  sleep 0.1
done
[ "$READY" = "1" ] || { echo "axis_ini did not become ready"; exit 1; }

request /zmot/api/open_device '{}'
assert_zero 'initial session'
request /zmot/api/session_status '' GET
assert_zero 'status before startup'
assert_contains '"ecat_running" *: *false' 'bus before startup'

request /zmot/api/eni_select "{\"remote_file\":\"$ENI_PATH\"}"
assert_zero 'select initial ENI'
request /zmot/api/startup
assert_zero 'ENI startup'
request /zmot/api/session_status '' GET
assert_zero 'status after startup'
assert_contains '"ecat_running" *: *true' 'bus after startup'
assert_contains "$ENI_PATH" 'current ENI after startup'

# Selection affects the next startup while the current configuration stays active.
request /zmot/api/eni_select "{\"remote_file\":\"$NEXT_ENI_PATH\"}"
assert_zero 'select next ENI'
request /zmot/api/session_status '' GET
assert_zero 'status after next selection'
assert_contains "$ENI_PATH" 'current ENI before restart'
request /zmot/api/selected_config_files '' GET
assert_zero 'next configuration'
assert_contains "$NEXT_ENI_PATH" 'next ENI before restart'
request /zmot/api/ecat_stop
assert_zero 'stop before restart'
request /zmot/api/session_status '' GET
assert_zero 'status after stop'
assert_contains '"ecat_running" *: *false' 'bus after stop'
request /zmot/api/startup_ini "{\"local_file\":\"$INI_PATH\"}"
assert_zero 'INI restart'
request /zmot/api/session_status '' GET
assert_zero 'status after restart'
assert_contains '"ecat_running" *: *true' 'bus after restart'
assert_contains "$NEXT_ENI_PATH" 'current ENI after restart'
request /zmot/api/ini_name '' GET
assert_zero 'INI after restart'
assert_contains "$INI_PATH" 'current INI after restart'

request /zmot/api/server_exit
assert_zero server_exit

TRIES=0
while [ "$TRIES" -lt 100 ]; do
  if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    wait "$SERVER_PID"
    SERVER_PID=""
    break
  fi
  TRIES=$((TRIES + 1))
  sleep 0.1
done

if [ "$SERVER_PID" != "" ]; then
  echo "axis_ini did not exit after server_exit"
  cat "$SERVER_OUT" "$SERVER_ERR" 2>/dev/null || true
  exit 1
fi

# Verify that a positional INI starts the bus through the executable entry point.
(cd "$REPO_ROOT" && exec "$MOTAPP" axis_ini "$INI_PATH" "--port=$HTTP_PORT" \
  > "$INI_SERVER_OUT" 2> "$INI_SERVER_ERR") &
SERVER_PID=$!

SESSION=""
READY=0
TRIES=0
while [ "$TRIES" -lt 150 ]; do
  request /zmot/api/open '{}'
  if grep -Eq 'Status: *200' "$RESPONSE_OUT" && grep -Eq '"code": *0' "$RESPONSE_OUT"; then
    READY=1
    break
  fi
  if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "axis_ini with INI exited before ready"
    cat "$INI_SERVER_OUT" "$INI_SERVER_ERR" 2>/dev/null || true
    exit 1
  fi
  TRIES=$((TRIES + 1))
  sleep 0.1
done
[ "$READY" = "1" ] || {
  echo "axis_ini with INI did not become ready after EtherCAT startup"
  exit 1
}

assert_zero 'preconfigured open'
request /zmot/api/session_status '' GET
assert_zero 'preconfigured status'
assert_contains '"ecat_running" *: *true' 'preconfigured bus'
request /zmot/api/server_exit
assert_zero 'preconfigured server_exit'

TRIES=0
while [ "$TRIES" -lt 100 ]; do
  if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    wait "$SERVER_PID"
    SERVER_PID=""
    trap - EXIT
    exit 0
  fi
  TRIES=$((TRIES + 1))
  sleep 0.1
done

echo "preconfigured axis_ini did not exit after server_exit"
cat "$INI_SERVER_OUT" "$INI_SERVER_ERR" 2>/dev/null || true
exit 1
