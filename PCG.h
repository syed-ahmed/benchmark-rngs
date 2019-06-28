#pragma once

// define constants like M_PI and C keywords for MSVC
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include <stdint.h>
#include <cmath>

namespace at {

class pcg_engine {
public:
  inline explicit pcg_engine(uint64_t initstate = 0x853c49e6748fea9bULL, uint64_t initseq = 0xda3e39cb94b95bdbULL) {
    rng.state = 0U;
    rng.inc = (initseq << 1u) | 1u;
    pcg32_random_r();
    rng.state += initstate;
    pcg32_random_r();
  }

  inline uint32_t operator()() {
      return pcg32_random_r();
  }

  void advance(uint64_t delta) {
    uint64_t cur_mult = PCG_DEFAULT_MULTIPLIER_64;
    uint64_t cur_plus = rng.inc;
    uint64_t acc_mult = 1u;
    uint64_t acc_plus = 0u;
    while (delta > 0) {
        if (delta & 1) {
            acc_mult *= cur_mult;
            acc_plus = acc_plus * cur_mult + cur_plus;
        }
        cur_plus = (cur_mult + 1) * cur_plus;
        cur_mult *= cur_mult;
        delta /= 2;
    }
    rng.state = acc_mult * rng.state + acc_plus;
  }

private:
    typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;
    pcg32_random_t rng;
    static const uint64_t PCG_DEFAULT_MULTIPLIER_64 = 6364136223846793005ULL;


    uint64_t pcg32_random_r() {
        uint64_t oldstate = rng.state;
        rng.state = oldstate * PCG_DEFAULT_MULTIPLIER_64 + rng.inc;
        uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        uint32_t rot = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }
};

typedef pcg_engine pcg;

} // namespace at