#include "Philox.h"
#include "PhiloxSIMD.h"
#include "MT19937.h"
#include "CLI11.hpp"
#include "fort.hpp"
#include <iostream>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>
#include <limits>
#include <stdint.h>
#include <x86intrin.h>
#include <stdexcept>
#include <cmath>
#include <string>
#include <map>
#include <cstring>

using namespace at;

typedef std::string (*benchmark_function)(std::string name, fort::table& results_table, uint64_t loop_count, uint64_t num_threads);
typedef std::map<std::string, benchmark_function> benchmarks_map_t;

std::string philox_global_instance(std::string name, fort::table& results_table, uint64_t loop_count, uint64_t num_threads);
std::string philox_thread_local_instance(std::string name, fort::table& results_table, uint64_t loop_count, uint64_t num_threads);
std::string philox_simd_global_instance(std::string name, fort::table& results_table, uint64_t loop_count, uint64_t num_threads);
std::string philox_simd_thread_local_instance(std::string name, fort::table& results_table, uint64_t loop_count, uint64_t num_threads);
std::string at_mt19937(std::string name, fort::table& results_table, uint64_t loop_count, uint64_t num_threads);
std::string at_pcg(std::string name, fort::table& results_table, uint64_t loop_count, uint64_t num_threads);
std::string std_mt19937(std::string name, fort::table& results_table, uint64_t loop_count, uint64_t num_threads);
std::string xoshiro256(std::string name, fort::table& results_table, uint64_t loop_count, uint64_t num_threads);
void check_philox_vs_simd();

void run_benchmark_suite(fort::table& results_table, benchmarks_map_t benchmarks, uint64_t loop_count, uint64_t num_threads) {
    for (auto const& x : benchmarks) {
        results_table << (*(x.second))(x.first, results_table, loop_count, num_threads);
    }
    results_table << fort::endr;
}

int main(int argc, char **argv){
    CLI::App app{"Random Number Engine Benchmark"};
    auto loop_count = 134217728UL;
    auto num_threads = 1;
    bool benchmark_increasing_threads = false;
    bool benchmark_increasing_loop_count = false;
    auto benchmark_metric = app.add_option_group("benchmark_type", 
                                                 "Decides if the independent variable is number of threads or number of randoms");
    benchmark_metric->add_option("--benchmark-threads", benchmark_increasing_threads,
                                 "Run full benchmark with increasing number of threads");
    benchmark_metric->add_option("--benchmark-loop-count", benchmark_increasing_loop_count,
                                 "Run full benchmark with increasing number of randoms");
    benchmark_metric->require_option(0, 1); // require at most one of the benchmark_type options
    app.add_option("-l,--loop-count", loop_count, "Number of randoms to produce");
    app.add_option("-t,--num-threads", num_threads, "Number of threads");
    CLI11_PARSE(app, argc, argv);

    // Add benchmarks
    benchmarks_map_t m;
    m.emplace("philox (global)", &philox_global_instance);
    m.emplace("philox (thread local)", &philox_simd_global_instance);
    m.emplace("philox_simd (global)", &philox_thread_local_instance);
    m.emplace("philox_simd (thread local)", &philox_simd_thread_local_instance);
    m.emplace("xoshiro256**", &xoshiro256);
    m.emplace("pcg64", &at_pcg);
    m.emplace("at::mt19937", &at_mt19937);
    m.emplace("std::mt19937", &std_mt19937);

    fort::table results_table;
    
    if (benchmark_increasing_threads) {
        if((loop_count & (loop_count - 1)) != 0){
            throw std::runtime_error("Number of randoms must be a power of 2");
        }

        results_table << fort::header;
        results_table << "Number of Threads";
        for (auto const& x : m) {
            results_table << x.first;
        }
        results_table << fort::endr;

        for(int i = 0; i < 8; i++) {
            auto thread_count = 1 << i;
            results_table << std::to_string(thread_count);
            run_benchmark_suite(results_table, m, loop_count, thread_count);
        }
        std::cout << std::endl;
        std::cout << "Summary: " << "Time (seconds) to get " + std::to_string(loop_count) + " randoms with varying number of threads" << std::endl;
    } else if (benchmark_increasing_loop_count) {
        if((num_threads & (num_threads - 1)) != 0){
            throw std::runtime_error("Number of threads must be a power of 2");
        }
        results_table << fort::header;
        results_table << "Number of Randoms";
        for (auto const& x : m) {
            results_table << x.first;
        }
        results_table << fort::endr;

        for(int i = 1; i < 10; i++) {
            auto num_randoms = 1 << 3*i;
            results_table << std::to_string(num_randoms);
            run_benchmark_suite(results_table, m, num_randoms, num_threads);
        }
        std::cout << std::endl;
        std::cout << "Summary: " << "Time (seconds) for " + std::to_string(num_threads) + " threads to get varying number of randoms" << std::endl;
    } else {
        if(loop_count % num_threads != 0){
            throw std::runtime_error("Number of randoms to produce has to be divisible by number of threads");
        }
        results_table << fort::header;
        for (auto const& x : m) {
            results_table << x.first;
        }

        results_table << fort::endr;
        run_benchmark_suite(results_table, m, loop_count, num_threads);
        std::cout << std::endl;
        std::cout << "Summary: " << "Time (seconds) to get " + std::to_string(loop_count) + " randoms with " + std::to_string(num_threads) + " threads" << std::endl;
    }

    // table format
    results_table.row(0).set_cell_text_align(fort::text_align::center);
    std::cout << results_table.to_string() << std::endl;

    // check_philox_vs_simd();  // NOTE: must set UNSHUFFLE to 1 for generators to match exactly
}
