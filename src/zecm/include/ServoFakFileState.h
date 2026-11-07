#ifndef _ZMC_SERVO_FAK_FILE_STATE_H_
#define _ZMC_SERVO_FAK_FILE_STATE_H_

#include "stdint.h"

namespace zmc {

class ServoSgFaker;

// Enabled when state file path is resolved from (in order):
//   1) getenv("ZECM_SERVO_FAK_STATE_FILE")
//   2) getenv("ZECM_SERVO_FAK_ENV_FILE") or ./zecm_servo_fak.env (KEY=VALUE)
bool servo_fak_state_enabled();

// Merge string into extra object (used before save; persisted across load/save).
void servo_fak_state_set_extra_string(const char* key, const char* value);

// Call once per ECAT PDO cycle (after all servo runCycle in that cycle).
// Increments tick when state file is enabled; save writes current tick (cycle count).
void servo_fak_state_on_cycle();

// Login: read last valid JSONL line and apply sparse servos[i] for 0 <= i < count.
// Returns 0 on success, -1 if disabled or no usable snapshot.
int servo_fak_state_load(ServoSgFaker* svArr, int count, double cycleTimeSec);

// Logout: append one JSONL snapshot line (servos 0..count-1 + extra).
// Returns 0 on success, -1 if disabled or write failed.
int servo_fak_state_save(const ServoSgFaker* svArr, int count);

}  // namespace zmc

#endif
