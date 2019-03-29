#include "Philox.h"
#include "MT19937.h"
#include <iostream>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>
#include <limits>

void philox(uint64_t loop_count);
void at_mt19937(uint64_t loop_count);
void std_mt19937(uint64_t loop_count);
void std_mt19937_at_uniform(uint64_t loop_count);

int main(){
    auto loop_count = 100000000UL;
    philox(loop_count);
    at_mt19937(loop_count);
    std_mt19937(loop_count);
    std_mt19937_at_uniform(loop_count);
}
