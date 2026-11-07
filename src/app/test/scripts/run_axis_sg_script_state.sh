#!/usr/bin/env sh
set -e
if [ "$#" -lt 3 ]; then
  echo "Usage: run_axis_sg_script_state.sh <motapp_demo_exe> <fixture_bin_dir> <zcode_path>"
  exit 1
fi

MOTAPP="$1"
FIXTURE_BIN="$2"
ZCODE="$3"
# Keep paths usable by shell tools and native Windows executables. GPT-6.
if command -v cygpath >/dev/null 2>&1; then
  MOTAPP=$(cygpath -m "$MOTAPP")
  FIXTURE_BIN=$(cygpath -m "$FIXTURE_BIN")
  ZCODE=$(cygpath -m "$ZCODE")
fi
CASE_DIR="$FIXTURE_BIN/case_axis_sg_script_state"
SEED="$CASE_DIR/servo_fak_seed.jsonl"
JSONL="$CASE_DIR/servo_fak.jsonl"

if [ ! -x "$MOTAPP" ] && [ ! -f "$MOTAPP" ]; then
  echo "motapp demo not found: $MOTAPP"
  exit 1
fi
if [ ! -f "$ZCODE" ]; then
  echo "zcode not found: $ZCODE"
  exit 1
fi
if [ ! -f "$SEED" ]; then
  echo "seed jsonl not found: $SEED"
  exit 1
fi

count_lines() {
  if [ ! -f "$1" ] || [ ! -s "$1" ]; then
    echo 0
    return
  fi
  wc -l < "$1" | tr -d ' '
}

mkdir -p "$CASE_DIR"
cp "$SEED" "$JSONL"

BEFORE=$(count_lines "$JSONL")
export ZECM_SERVO_FAK_STATE_FILE="$JSONL"
"$MOTAPP" axis_sg --cmd --script "$ZCODE" --script_exit
AFTER=$(count_lines "$JSONL")

if [ "$BEFORE" = "$AFTER" ]; then
  echo "axis_sg_script_state: line count did not change before=$BEFORE after=$AFTER"
  exit 1
fi

LAST=$(tail -n 1 "$JSONL")
if [ -z "$LAST" ]; then
  echo "axis_sg_script_state: last snapshot is empty"
  exit 1
fi

printf '%s\n' "$LAST" | grep -q '"version":1' || {
  echo "axis_sg_script_state: expected version 1 in last snapshot"
  exit 1
}
printf '%s\n' "$LAST" | grep -q '"servos"' || {
  echo "axis_sg_script_state: expected servos object in last snapshot"
  exit 1
}
printf '%s\n' "$LAST" | grep -q '"0"' || {
  echo "axis_sg_script_state: expected axis 0 in last snapshot"
  exit 1
}
printf '%s\n' "$LAST" | grep -q '"opmode":8' || {
  echo "axis_sg_script_state: expected opmode 8 in last snapshot"
  exit 1
}
printf '%s\n' "$LAST" | grep -q '"vel":0' || {
  echo "axis_sg_script_state: expected vel 0 in last snapshot"
  exit 1
}
if printf '%s\n' "$LAST" | grep -q '"pos":1'; then
  echo "axis_sg_script_state: last snapshot still has seed position 1"
  exit 1
fi
if printf '%s\n' "$LAST" | grep -q '"statusword":567'; then
  echo "axis_sg_script_state: last snapshot still has seed statusword 567"
  exit 1
fi

exit 0
