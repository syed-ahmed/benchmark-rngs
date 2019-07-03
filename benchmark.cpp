#include "Philox.h"
#include "PhiloxSIMD.h"
#include "MT19937.h"
#include <iostream>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>
#include <limits>
#include <stdint.h>
#include <x86intrin.h>

using namespace at;

void philox(uint64_t loop_count);
void philox_simd(uint64_t loop_count);
void at_mt19937(uint64_t loop_count);
void std_mt19937(uint64_t loop_count);
void std_mt19937_at_uniform(uint64_t loop_count);
void xoshiro256(uint64_t loop_count);
void pcg64(uint64_t loop_count);
void check_philox_vs_simd();

int main(){
    auto loop_count = 100000000UL;

    // check_philox_vs_simd();  // NOTE: must set UNSHUFFLE to 1 for generators to match exactly

    philox(loop_count);
    philox_simd(loop_count);
    xoshiro256(loop_count);
    pcg64(loop_count);
    at_mt19937(loop_count);
    std_mt19937(loop_count);

    // std_mt19937_at_uniform(loop_count);
}
