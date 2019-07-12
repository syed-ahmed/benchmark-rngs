#include "Philox.h"
#include "MT19937.h"
#include "xoshiro256starstar.h"
#include "PhiloxSIMD.h"
#include "PCG.h"
#include "fort.hpp"
#include <iostream>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>
#include <limits>
#include <thread>
#include <vector>
#include <string>

constexpr int TRIALS = 3;
constexpr float POW_2_32_INV = 1.0f/std::numeric_limits<uint32_t>::max();

float to_float_from_uint(uint32_t x) {
    float res = x * POW_2_32_INV;
    if (res == static_cast<float>(1.0)) {
      res = std::nextafter(static_cast<float>(1.0), static_cast<float>(0.0));
    }
    return res;
}

template <typename func_t>
static std::string benchmark(std::string name, uint64_t loop_count, const func_t& f, uint64_t num_threads) {
    std::cout << std::endl;
    std::cout << "Benchmarking: " << name << std::endl;
    double best = std::numeric_limits<double>::infinity();
    for (int trial = 0; trial < TRIALS; trial++) {
        std::vector<std::thread> threads;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_threads; ++i) {
            threads.push_back(std::thread(f, i, trial));
        }
        for(auto &t : threads) {
            t.join();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        best = std::min(best, diff.count());
    }
    std::cout << "Time to get " << loop_count << " " << name << " randoms, with " << num_threads << " thread(s) = " << best << " s" << std::endl;
    return std::to_string(best);
}

void check_philox_vs_simd() {
    at::philox_engine philox(0,0,0);
    at::philox_simd_engine philox_simd(0,0,0);
    bool ok = true;
    for (int i = 0; i < 1000; i++) {
        uint32_t expected = philox();
        uint32_t actual = philox_simd();
        if (expected != actual) {
            printf("philox differs from philox_simd at %d (%08x vs %08x)\n", i, expected, actual);
            ok = false;
            break;
        }
    }
    if (ok) {
        printf("OK\n");
    }
}

std::string philox_global_instance(std::string name, fort::table& results_table, uint64_t loop_count = 134217728UL, uint64_t num_threads=1) {
    uint32_t y = 0;
    at::philox_engine gen1(0,0,0);
    std::mutex mutex;
    uint64_t step = 128 / 32;
    auto bench = benchmark(name, loop_count, [&] (uint64_t thread_idx, int trial) {
        std::lock_guard<std::mutex> lock(mutex);
        for(uint64_t i = 0; i < loop_count / num_threads; i+=step) {
            auto z = gen1.next();
            y += z[0];
            y += z[1];
            y += z[2];
            y += z[3];
        }
    }, num_threads);
    std::cout << "Accumulated Y value is " << y << std::endl;
    return bench;
}

std::string philox_thread_local_instance(std::string name, fort::table& results_table, uint64_t loop_count = 134217728UL, uint64_t num_threads=1) {
    uint64_t step = 128 / 32;
    auto bench = benchmark(name, loop_count, [&] (uint64_t thread_idx, int trial) {
        uint32_t y = 0;
        at::philox_engine gen1(0, thread_idx, 0);
        for(uint64_t i = 0; i < loop_count / num_threads; i+=step) {
            auto z = gen1.next();
            y += z[0];
            y += z[1];
            y += z[2];
            y += z[3];
        }
        if(trial == TRIALS - 1) {
            std::cout << "Accumulated Y value at thread_idx " << thread_idx << " is " << y << std::endl;
        }
    }, num_threads);
    return bench;
}

std::string philox_simd_global_instance(std::string name, fort::table& results_table, uint64_t loop_count = 134217728UL, uint64_t num_threads=1) {
    at::philox_simd_engine gen(0,0,0);
    std::mutex mutex;
    __m256i v = _mm256_set1_epi32(0);
    uint64_t step = 1024 / 32;
    auto bench = benchmark(name, loop_count, [&] (uint64_t thread_idx, int trial) {
        std::lock_guard<std::mutex> lock(mutex);
        for(uint64_t i = 0; i < loop_count / num_threads; i+=step) {
            __m256i a, b, c, d;
            gen.next32(a, b, c, d);
            v = _mm256_add_epi32(v, a);
            v = _mm256_add_epi32(v, b);
            v = _mm256_add_epi32(v, c);
            v = _mm256_add_epi32(v, d);
        }
    }, num_threads);
    uint32_t values[8];
    _mm256_storeu_si256((__m256i*)values, v);
    uint32_t y = 0;
    for (int i = 0; i < 8; i++) {
        y += values[i];
    }
    std::cout << "Accumulated Y value is " << y << std::endl;
    return bench;
}

std::string philox_simd_thread_local_instance(std::string name, fort::table& results_table, uint64_t loop_count = 134217728UL, uint64_t num_threads=1) {
    uint64_t step = 1024 / 32;
    auto bench = benchmark(name, loop_count, [&] (uint64_t thread_idx, int trial) {
        __m256i v = _mm256_set1_epi32(0);
        at::philox_simd_engine gen(0, thread_idx, 0);
        for(uint64_t i = 0; i < loop_count / num_threads; i+=step) {
            __m256i a, b, c, d;
            gen.next32(a, b, c, d);
            v = _mm256_add_epi32(v, a);
            v = _mm256_add_epi32(v, b);
            v = _mm256_add_epi32(v, c);
            v = _mm256_add_epi32(v, d);
        }
        uint32_t values[8];
        _mm256_storeu_si256((__m256i*)values, v);
        uint32_t y = 0;
        for (int i = 0; i < 8; i++) {
            y += values[i];
        }
        if(trial == TRIALS - 1) {
            std::cout << "Accumulated Y value at thread_idx " << thread_idx << " is " << y << std::endl;
        }
    }, num_threads);
    return bench;
}

std::string xoshiro256(std::string name, fort::table& results_table, uint64_t loop_count = 134217728UL, uint64_t num_threads=1) {
    uint64_t y = 0;
    xoshiro256starstar_engine gen1(0);
    std::mutex mutex;
    uint64_t step = 64 / 32;
    auto bench = benchmark(name, loop_count, [&] (uint64_t thread_idx, int trial) {
        std::lock_guard<std::mutex> lock(mutex);
        for(uint64_t i = 0; i < loop_count / num_threads; i+=step) {
            uint64_t z = gen1.next();
            y += z;
        }
    }, num_threads);
    std::cout << "Accumulated Y value is " << y << std::endl;
    return bench;
}

std::string at_pcg(std::string name, fort::table& results_table, uint64_t loop_count = 134217728UL, uint64_t num_threads=1) {
    uint64_t y = 0;
    at::pcg gen1;
    std::mutex mutex;
    uint64_t step = 64 / 32;
    auto bench = benchmark(name, loop_count, [&] (uint64_t thread_idx, int trial) {
        std::lock_guard<std::mutex> lock(mutex);
        for(uint64_t i = 0; i < loop_count / num_threads; i+=step) {
            y += gen1();
        }
    }, num_threads);
    std::cout << "Accumulated Y value is " << y << std::endl;
    return bench;
}

std::string at_mt19937(std::string name, fort::table& results_table, uint64_t loop_count = 134217728UL, uint64_t num_threads=1) {
    uint32_t y = 0;
    at::mt19937 gen1;
    std::mutex mutex;
    uint64_t step = 32 / 32;
    auto bench = benchmark(name, loop_count, [&] (uint64_t thread_idx, int trial) {
        std::lock_guard<std::mutex> lock(mutex);
        for(uint64_t i = 0; i < loop_count / num_threads; i+=step) {
            y += gen1();
        }
    }, num_threads);
    std::cout << "Accumulated Y value is " << y << std::endl;
    return bench;
}

std::string std_mt19937(std::string name, fort::table& results_table, uint64_t loop_count = 134217728UL, uint64_t num_threads=1) {
    uint32_t y = 0;
    std::mt19937 gen1;
    std::mutex mutex;
    uint64_t step = 32 / 32;
    auto bench = benchmark(name, loop_count, [&] (uint64_t thread_idx, int trial) {
        std::lock_guard<std::mutex> lock(mutex);
        for(uint64_t i = 0; i < loop_count / num_threads; i+=step) {
            y += gen1();
        }
    }, num_threads);
    std::cout << "Accumulated Y value is " << y << std::endl;
    return bench;
}

