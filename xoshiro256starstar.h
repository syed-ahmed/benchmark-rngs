#include <stdint.h>
#include "splitmix64.h"

// Modified from
// http://xoroshiro.di.unimi.it/xoshiro256starstar.c
// http://xoroshiro.di.unimi.it/splitmix64.c

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}


struct xoshiro256starstar_engine {

uint64_t s[4];

xoshiro256starstar_engine(uint64_t seed) {
 s[0] = splitmix64(seed);
 s[1] = splitmix64(seed);
 s[2] = splitmix64(seed);
 s[3] = splitmix64(seed);
}

uint64_t next(void) {
    const uint64_t result_starstar = rotl(s[1] * 5, 7) * 9;

    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 45);

    return result_starstar;
}

};
