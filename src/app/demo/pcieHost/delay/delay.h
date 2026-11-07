#ifndef MC_PCIEHOST_DELAY_H
#define MC_PCIEHOST_DELAY_H

/*
 * Minimal millisecond sleep helper used by mc_pdo_thread.
 * Linux-only build flavor; if Windows support is added later, switch on
 * _WIN32 to Sleep().
 */

#include <unistd.h>

static inline void delay_ms(unsigned int ms) {
    usleep((useconds_t)ms * 1000u);
}

#endif /* MC_PCIEHOST_DELAY_H */
