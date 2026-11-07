#ifndef ZMC_FNV1A128_H_
#define ZMC_FNV1A128_H_

#include <stddef.h>
#include <stdint.h>

typedef struct U128 {
    uint64_t high;
    uint64_t low;
} U128;

static inline void fnv1a_128_init(U128* state) {
    state->high = 0x6c62272e07bb0142ULL;
    state->low = 0x62b821756295c58dULL;
}

static inline void fnv1a_128_update(U128* state, const void* data, size_t len) {
    const uint8_t* p = (const uint8_t*)data;
    const uint64_t PRIME_LOW = 0x13B;

    for (size_t i = 0; i < len; ++i) {
        const uint64_t old_high = state->high;
        const uint64_t low = state->low ^ p[i];

        // Multiply the low 64-bit word by 0x13B using 32-bit limbs.
        // prime = 2^88 + 0x13B
        const uint64_t a_lo = (uint32_t)low;
        const uint64_t a_hi = low >> 32;

        const uint64_t p0 = a_lo * PRIME_LOW;
        const uint64_t p1 = a_hi * PRIME_LOW;

        const uint64_t p1_lo = p1 & 0xFFFFFFFFULL;
        const uint64_t p1_hi = p1 >> 32;

        const uint64_t sum_low = p0 + (p1_lo << 32);
        const uint64_t carry = (sum_low < p0) ? 1 : 0;

        // Include the old high word's product contribution modulo 2^128.
        state->high = old_high * PRIME_LOW + p1_hi + carry + (low << 24);
        state->low = sum_low;
    }
}

static inline void fnv1a_128_digest(const U128* state, char out[33]) {
    static const char digits[] = "0123456789abcdef";
    const uint64_t words[] = {state->high, state->low};
    for (size_t word = 0; word < 2; ++word) {
        for (size_t nibble = 0; nibble < 16; ++nibble) {
            const unsigned int shift = (unsigned int)((15 - nibble) * 4);
            out[word * 16 + nibble] = digits[(words[word] >> shift) & 0x0fU];
        }
    }
    out[32] = '\0';
}

static inline U128 fnv1a_128(const void* data, size_t len) {
    U128 result;
    fnv1a_128_init(&result);
    fnv1a_128_update(&result, data, len);
    return result;
}

#endif
