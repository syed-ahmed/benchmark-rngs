#include "Philox.h"
#include "PhiloxSIMD.h"
#include "MT19937.h"
#include "CLI11.hpp"
#include <iostream>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>
#include <limits>
#include <stdint.h>
#include <x86intrin.h>

using namespace at;

void philox_global_instance(uint64_t loop_count, uint64_t num_threads);
void philox_thread_local_instance(uint64_t loop_count, uint64_t num_threads);
void philox_simd_global_instance(uint64_t loop_count, uint64_t num_threads);
void philox_simd_thread_local_instance(uint64_t loop_count, uint64_t num_threads);
void at_mt19937(uint64_t loop_count, uint64_t num_threads);
void at_pcg(uint64_t loop_count, uint64_t num_threads);
void std_mt19937(uint64_t loop_count, uint64_t num_threads);
void std_mt19937_at_uniform(uint64_t loop_count, uint64_t num_threads);
void xoshiro256(uint64_t loop_count, uint64_t num_threads);
void check_philox_vs_simd();

int main(int argc, char **argv){
    CLI::App app{"Random Number Engine Benchmark"};
    auto loop_count = 100000000UL;
    auto num_threads = 1;
    app.add_option("-l,--loop-count", loop_count, "Number of randoms to produce per thread");
    app.add_option("-t,--num-threads", num_threads, "Number of threads");
    CLI11_PARSE(app, argc, argv);

    // check_philox_vs_simd();  // NOTE: must set UNSHUFFLE to 1 for generators to match exactly

    philox_global_instance(loop_count, num_threads);
    philox_simd_global_instance(loop_count, num_threads);
    philox_thread_local_instance(loop_count, num_threads);
    philox_simd_thread_local_instance(loop_count, num_threads);
    xoshiro256(loop_count, num_threads);
    at_pcg(loop_count, num_threads);
    at_mt19937(loop_count, num_threads);
    std_mt19937(loop_count, num_threads);
}
