#include "Philox.h"
#include "MT19937.h"
#include <iostream>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>
#include <limits>

void philox(uint64_t loop_count = 1000000000UL) {
    float x = 0;
    at::philox_engine gen1(0,0,0);
    auto start = std::chrono::high_resolution_clock::now();
    for(uint64_t i = 0; i < loop_count; i++) {
        x = gen1() * 123;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to get philox random " << diff.count() << " s\n";
    std::cout << "X value is " << x << std::endl;
}

void at_mt19937(uint64_t loop_count = 1000000000UL) {
    float x = 0;
    at::mt19937 gen1;
    auto start = std::chrono::high_resolution_clock::now();
    for(uint64_t i = 0; i < loop_count; i++) {
        x = gen1() * 123;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to get at::mt19937 random " << diff.count() << " s\n";
    std::cout << "X value is " << x << std::endl;
}

void std_mt19937(uint64_t loop_count = 1000000000UL) {
    float x = 0;
    std::mt19937 gen1;
    auto start = std::chrono::high_resolution_clock::now();
    for(uint64_t i = 0; i < loop_count; i++) {
        x = gen1() * 123;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Time to get std::mt19937 Random " << diff.count() << " s\n";
    std::cout << "X value is " << x << std::endl;
}

int main(){
    philox(100000000UL);
    std_mt19937(100000000UL);
    at_mt19937(100000000UL);
}
