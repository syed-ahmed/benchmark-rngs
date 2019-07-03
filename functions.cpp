#include "Philox.h"
#include "MT19937.h"
#include "xoshiro256starstar.h"
#include "PhiloxSIMD.h"
#include "pcg64.h"
#include <iostream>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>
#include <limits>

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
static void benchmark(const char* name, uint64_t loop_count, uint64_t bits, const func_t& f) {
    double best = std::numeric_limits<double>::infinity();
    uint64_t step = bits / 32;

    for (int trial = 0; trial < TRIALS; trial++) {
        auto start = std::chrono::high_resolution_clock::now();
        for(uint64_t i = 0; i < loop_count; i+=step) {
            f();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        best = std::min(best, diff.count());
    }
    std::cout << "Time to get " << loop_count << " " << name << " randoms = " << best << "s" << std::endl;
}

void philox(uint64_t loop_count = 1000000000UL) {
    uint32_t y = 0;
    at::philox_engine gen1(0,0,0);
    benchmark("philox", loop_count, 128, [&]{
        auto z = gen1.next();
        y += z[0];
        y += z[1];
        y += z[2];
        y += z[3];
    });
    std::cout << "Y value is " << y << std::endl;
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

void philox_simd(uint64_t loop_count = 1000000000UL) {
    at::philox_simd_engine gen(0,0,0);
    __m256i v;
    benchmark("philox_simd", loop_count, 1024, [&]{
        __m256i a, b, c, d;
        gen.next32(a, b, c, d);
        v = _mm256_add_epi32(v, a);
        v = _mm256_add_epi32(v, b);
        v = _mm256_add_epi32(v, c);
        v = _mm256_add_epi32(v, d);
    });
    uint32_t values[8];
    _mm256_storeu_si256((__m256i*)values, v);
    uint32_t y = 0;
    for (int i = 0; i < 8; i++) {
        y += values[i];
    }
    std::cout << "Y value is " << y << std::endl;
}

void xoshiro256(uint64_t loop_count = 1000000000UL) {
    uint64_t y = 0;
    xoshiro256starstar_engine gen1(0);
    benchmark("xoshiro256**", loop_count, 64, [&]{
        uint64_t z = gen1.next();
        y += z;
    });
    std::cout << "Y value is " << y << std::endl;
}

void pcg64(uint64_t loop_count = 1000000000UL) {
    uint64_t y = 0;
    pcg64_random_t gen = PCG64_INITIALIZER;
    benchmark("pcg64", loop_count, 64, [&]{
        y += pcg64_random_r(&gen);
    });
    std::cout << "Y value is " << y << std::endl;
}

void at_mt19937(uint64_t loop_count = 1000000000UL) {
    uint32_t y = 0;
    at::mt19937 gen1;
    benchmark("at::mt19937", loop_count, 32, [&]{
        y += gen1();
    });
    std::cout << "Y value is " << y << std::endl;
}

void std_mt19937(uint64_t loop_count = 1000000000UL) {
    uint32_t y = 0;
    std::mt19937 gen1;
    benchmark("std::mt19937", loop_count, 32, [&]{
        y += gen1();
    });
    std::cout << "Y value is " << y << std::endl;
}

// void std_mt19937_at_uniform(uint64_t loop_count = 1000000000UL) {
//     float x = 0;
//     std::mt19937 gen1;
//     //std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
//     auto start = std::chrono::high_resolution_clock::now();
//     for(uint64_t i = 0; i < loop_count; i++) {
//         x = to_float_from_uint(gen1());
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> diff = end-start;
//     std::cout << "Time to get " << loop_count << " std::mt19937 randoms with at::uniform_real_distribution = " << diff.count() << "s" << std::endl;
//     std::cout << "X value is " << x << std::endl;
// }
