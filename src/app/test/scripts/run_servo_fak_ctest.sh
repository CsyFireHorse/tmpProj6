#!/usr/bin/env sh
set -e
if [ "$#" -lt 3 ]; then
  echo "Usage: run_servo_fak_ctest.sh <motapp_exe> <fixture_bin_dir> <case_name>"
  exit 1
fi

MOTAPP="$1"
FIXTURE_BIN="$2"
CASE="$3"

# Keep paths usable by shell tools and native Windows executables. GPT-6.
if command -v cygpath >/dev/null 2>&1; then
  MOTAPP=$(cygpath -m "$MOTAPP")
  FIXTURE_BIN=$(cygpath -m "$FIXTURE_BIN")
fi

if [ ! -x "$MOTAPP" ] && [ ! -f "$MOTAPP" ]; then
  echo "motapp not found: $MOTAPP"
  exit 1
fi

count_lines() {
  if [ ! -f "$1" ] || [ ! -s "$1" ]; then
    echo 0
    return
  fi
  wc -l < "$1" | tr -d ' '
}

case "$CASE" in
  env_file)
    export ZECM_SERVO_FAK_ENV_FILE="$FIXTURE_BIN/case_env_file/zecm_servo_fak.env"
    exec "$MOTAPP" --gtest_filter=ServoFakFileState.EnvFileEnabled
    ;;
  env_override)
    export ZECM_SERVO_FAK_ENV_FILE="$FIXTURE_BIN/case_env_override/zecm_servo_fak.env"
    export ZECM_SERVO_FAK_STATE_FILE="$FIXTURE_BIN/case_env_override/from_process.jsonl"
    exec "$MOTAPP" --gtest_filter=ServoFakFileState.ProcessEnvWins
    ;;
  save_append)
    JSONL="$FIXTURE_BIN/case_save_append/servo_fak.jsonl"
    mkdir -p "$FIXTURE_BIN/case_save_append"
    if [ ! -f "$JSONL" ]; then
      : > "$JSONL"
    fi
    export ZECM_SERVO_FAK_STATE_FILE="$JSONL"
    BEFORE=$(count_lines "$JSONL")
    "$MOTAPP" --gtest_filter=ServoFakFileState.SaveWritesSnapshot
    AFTER=$(count_lines "$JSONL")
    if [ "$BEFORE" = "$AFTER" ]; then
      echo "save_append: line count did not change before=$BEFORE after=$AFTER"
      exit 1
    fi
    if ! grep -q '"pos":4242' "$JSONL"; then
      echo "save_append: expected pos 4242 in snapshot"
      exit 1
    fi
    exit 0
    ;;
  *)
    echo "unknown case: $CASE"
    exit 1
    ;;
esac
