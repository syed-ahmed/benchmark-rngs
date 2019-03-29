#include "Philox.h"
#include "MT19937.h"
#include <iostream>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>
#include <limits>

constexpr float POW_2_32_INV = 1.0f/std::numeric_limits<uint32_t>::max();

float to_float_from_uint(uint32_t x) {
    float res = x * POW_2_32_INV;
    if (res == static_cast<float>(1.0)) {
      res = std::nextafter(static_cast<float>(1.0), static_cast<float>(0.0));
    }
    return res;
}

void philox(uint64_t loop_count = 1000000000UL) {
    float x = 0;
    at::philox_engine gen1(0,0,0);
    auto start = std::chrono::high_resolution_clock::now();
    for(uint64_t i = 0; i < loop_count; i++) {
        x = to_float_from_uint(gen1());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to get " << loop_count << " philox randoms with at::uniform_real_distribution = " << diff.count() << "s" << std::endl;
    std::cout << "X value is " << x << std::endl;
}

void at_mt19937(uint64_t loop_count = 1000000000UL) {
    float x = 0;
    at::mt19937 gen1;
    auto start = std::chrono::high_resolution_clock::now();
    for(uint64_t i = 0; i < loop_count; i++) {
        x = to_float_from_uint(gen1());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to get " << loop_count << " at::mt19937 randoms with at::uniform_real_distribution = " << diff.count() << "s" << std::endl;
    std::cout << "X value is " << x << std::endl;
}

void std_mt19937(uint64_t loop_count = 1000000000UL) {
    float x = 0;
    std::mt19937 gen1;
    std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
    auto start = std::chrono::high_resolution_clock::now();
    for(uint64_t i = 0; i < loop_count; i++) {
        x = uniform(gen1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to get " << loop_count << " std::mt19937 randoms with std::uniform_real_distribution = " << diff.count() << "s" << std::endl;
    std::cout << "X value is " << x << std::endl;
}

void std_mt19937_at_uniform(uint64_t loop_count = 1000000000UL) {
    float x = 0;
    std::mt19937 gen1;
    //std::uniform_real_distribution<float> uniform(0.0f, 1.0f);
    auto start = std::chrono::high_resolution_clock::now();
    for(uint64_t i = 0; i < loop_count; i++) {
        x = to_float_from_uint(gen1());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to get " << loop_count << " std::mt19937 randoms with at::uniform_real_distribution = " << diff.count() << "s" << std::endl;
    std::cout << "X value is " << x << std::endl;
}