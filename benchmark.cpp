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
#include <vector>
#include <cstring>
#include <utility>
#include <functional>
#include <random>
#include <algorithm>
#include <iterator>
#include <fstream>

using namespace at;

typedef std::tuple<double, double, double, double> (*benchmark_function)(std::string name, uint64_t num_randoms, uint64_t num_threads);
typedef std::vector<std::tuple<double, double, double, double>> y_data_t;
typedef std::vector<std::pair<uint64_t, uint64_t>> x_data_t;
typedef std::vector<std::tuple<std::string, benchmark_function, y_data_t>> benchmarks_map_t;

std::tuple<double, double, double, double> philox_global_instance(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> philox_thread_local_instance(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> philox_simd_global_instance(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> philox_simd_thread_local_instance(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> at_mt19937(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> at_pcg(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> std_mt19937(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> xoshiro256(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> at_mt19937_chunking(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> at_pcg_chunking(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> std_mt19937_chunking(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> xoshiro256_chunking(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> philox_global_instance_chunking(std::string name, uint64_t num_randoms, uint64_t num_threads);
std::tuple<double, double, double, double> philox_simd_global_instance_chunking(std::string name, uint64_t num_randoms, uint64_t num_threads);
void check_philox_vs_simd();

void run_benchmark_suite(benchmarks_map_t& benchmarks, uint64_t num_randoms, uint64_t num_threads) {
    for (auto& x : benchmarks) {
        auto y_val = (*(std::get<1>(x)))(std::get<0>(x), num_randoms, num_threads);
        std::get<2>(x).emplace_back(y_val);   
    }
}

int main(int argc, char **argv){
    // Add benchmarks
    benchmarks_map_t tests_registry;
    tests_registry.emplace_back(std::make_tuple("philox (global)", &philox_global_instance, y_data_t()));
    tests_registry.emplace_back(std::make_tuple("philox (thread local)", &philox_thread_local_instance, y_data_t()));
    tests_registry.emplace_back(std::make_tuple("philox_simd (global)", &philox_simd_global_instance, y_data_t()));
    tests_registry.emplace_back(std::make_tuple("philox_simd (thread local)", &philox_simd_thread_local_instance, y_data_t()));
    tests_registry.emplace_back(std::make_tuple("xoshiro256**", &xoshiro256, y_data_t()));
    tests_registry.emplace_back(std::make_tuple("pcg64", &at_pcg, y_data_t()));
    tests_registry.emplace_back(std::make_tuple("at::mt19937", &at_mt19937, y_data_t()));
    tests_registry.emplace_back(std::make_tuple("std::mt19937", &std_mt19937, y_data_t()));
    // tests_registry.emplace_back(std::make_tuple("at::mt19937 (chunking)", &at_mt19937_chunking, y_data_t()));
    // tests_registry.emplace_back(std::make_tuple("pcg64 (chunking)", &at_pcg_chunking, y_data_t()));
    // tests_registry.emplace_back(std::make_tuple("std::mt19937 (chunking)", &std_mt19937_chunking, y_data_t()));
    // tests_registry.emplace_back(std::make_tuple("xoshiro256** (chunking)", &xoshiro256_chunking, y_data_t()));
    // tests_registry.emplace_back(std::make_tuple("philox (global) (chunking)", &philox_global_instance_chunking, y_data_t()));
    // tests_registry.emplace_back(std::make_tuple("philox_simd (global) (chunking)", &philox_simd_global_instance_chunking, y_data_t()));

    std::string engine_index_help_string = "Selects specific tests to run by supplying an int. Runs with all tests if none provided.\nAvailable tests are:";
    for(uint64_t i = 0; i < tests_registry.size(); ++i) {
        engine_index_help_string += "\n{" + std::to_string(i) + ": " + std::get<0>(tests_registry[i]) + "}";
    }

    CLI::App app{"Random Number Engine Benchmark"};
    uint64_t num_randoms = 134217728UL;
    auto num_threads = 1;
    auto max_num_x_data = 9;
    std::vector<uint32_t> engine_index;

    auto benchmark_metric = app.add_option_group("benchmark_type", 
                                                 "Decides if the independent variable is number of threads or number of randoms");
    auto benchmark_increasing_threads = benchmark_metric->add_flag("-a,--benchmark-threads", "Run full benchmark with increasing number of threads");
    auto benchmark_increasing_num_randoms = benchmark_metric->add_flag("-b,--benchmark-num-randoms", "Run full benchmark with increasing number of randoms");
    
    benchmark_metric->require_option(0, 1); // require at most one of the benchmark_type options
    app.add_option("-r,--min-num-randoms", num_randoms, "Minimum number of randoms to produce");
    app.add_option("-t,--min-num-threads", num_threads, "Minimum number of threads to use");
    app.add_option("-e,--engine", engine_index, engine_index_help_string);
    app.add_option("-x,--num-x-data-points", max_num_x_data, "Bins of x data points to produce, where x is either threads or number of randoms");
    CLI11_PARSE(app, argc, argv);
    
    // filter tests
    benchmarks_map_t m;
    if (!engine_index.empty()) {
        for (auto const& i : engine_index) {
            if(i > tests_registry.size() || i < 0) {
                throw std::runtime_error("Engine index is invalid. Please provide a valid test index for the -e,--engine flag");
            }
            m.emplace_back(tests_registry[i]);
        }
    } else {
        m = tests_registry;
    }

    // shuffle
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m.begin(), m.end(), g);

    std::cout << "Benchmarking with 3 trials. Modify TRIALS in functions.cpp if needed." << std::endl;
    x_data_t x_data;
    std::string file_name("result");
    if (benchmark_increasing_threads->count() > 0) {
        if((num_randoms == 0) || (num_randoms & (num_randoms - 1)) != 0){
            throw std::runtime_error("Number of randoms must be a power of 2");
        }
        file_name += "-increasing-threads";
        for(int i = 0; i < max_num_x_data; i++) {
            // shuffle when sampling new number of threads
            std::shuffle(m.begin(), m.end(), g);
            auto thread_count = 1ULL << i;
            x_data.emplace_back(std::make_pair(num_randoms, thread_count));
            run_benchmark_suite(m, num_randoms, thread_count);
        }
        std::cout << std::endl;
        std::cout << "Copy-paste the following in a markdown page for better readability" << std::endl;
        std::cout << "#### Summary: " << "Time (seconds) to get " + std::to_string(num_randoms) + " randoms with varying number of threads" << std::endl;
    } else if (benchmark_increasing_num_randoms->count() > 0) {
        if((num_threads == 0) || (num_threads & (num_threads - 1)) != 0){
            throw std::runtime_error("Number of threads must be a power of 2");
        }
        file_name += "-increasing-randoms";
        for(int i = 1; i <= max_num_x_data; i++) {
            // shuffle when sampling new number of randoms
            std::shuffle(m.begin(), m.end(), g);
            uint64_t num_randoms_inc = 1ULL << 3*i;
            x_data.emplace_back(std::make_pair(num_randoms_inc, num_threads));
            run_benchmark_suite(m, num_randoms_inc, num_threads);
        }
        std::cout << std::endl;
        std::cout << "Copy-paste the following in a markdown page for better readability" << std::endl;
        std::cout << "#### Summary: " << "Time (seconds) for " + std::to_string(num_threads) + " threads to get varying number of randoms" << std::endl;
    } else {
        if(num_randoms % num_threads != 0){
            throw std::runtime_error("Number of randoms to produce has to be divisible by number of threads");
        }
        // shuffle
        std::shuffle(m.begin(), m.end(), g);
        x_data.emplace_back(std::make_pair(num_randoms, num_threads));
        run_benchmark_suite(m, num_randoms, num_threads);
        std::cout << std::endl;
        std::cout << "Copy-paste the following in a markdown page for better readability" << std::endl;
        std::cout << "#### Summary: " << "Time (seconds) to get " + std::to_string(num_randoms) + " randoms with " + std::to_string(num_threads) + " threads" << std::endl;
    }

    std::cout << "##### Best and Worst Average Times per Thread" << std::endl;

    // print summary, write to file
    fort::table results_table_avg;
    std::ofstream file;
    file.open(file_name+"-avg.txt", std::ofstream::out | std::ofstream::trunc);

    results_table_avg << fort::header;
    results_table_avg << "Number of Randoms" << "Number of Threads";
    file << std::fixed << std::setprecision(12) << "Number of Randoms,Number of Threads";
    for (auto const& x : m) {
        auto test_name = std::get<0>(x);
        results_table_avg << test_name + " [best avg (s)]" << test_name + " [worst avg (s)]";
        file << "," << test_name + " [best avg (s)]," << test_name + " [worst avg (s)]";
    }
    file << "\n";
    results_table_avg << fort::endr;
    for(uint64_t i = 0; i < x_data.size(); i++) {
        results_table_avg << x_data[i].first << x_data[i].second;
        file << x_data[i].first << ","  << x_data[i].second;
        for (auto const& x : m) {
            auto y_data = std::get<2>(x);
            results_table_avg << std::get<0>(y_data[i]) << std::get<1>(y_data[i]);
            file << "," << std::get<0>(y_data[i]) << "," << std::get<1>(y_data[i]);
        }
        results_table_avg << fort::endr;
        file << "\n";
    }
    file.close();

    // table format
    results_table_avg.row(0).set_cell_text_align(fort::text_align::center);
    std::cout << results_table_avg.to_string() << std::endl;
    
    std::cout << "##### Best and Worst Max Times" << std::endl;

    // print summary, write to file
    fort::table results_table_max;
    file.open(file_name+"-max.txt", std::ofstream::out | std::ofstream::trunc);

    results_table_max << fort::header;
    results_table_max << "Number of Randoms" << "Number of Threads";
    file << std::fixed << std::setprecision(12) << "Number of Randoms,Number of Threads";
    for (auto const& x : m) {
        auto test_name = std::get<0>(x);
        results_table_max << test_name + " [best max (s)]" << test_name + " [worst max (s)]";
        file << "," << test_name + " [best max (s)]," << test_name + " [worst max (s)]";
    }
    file << "\n";
    results_table_max << fort::endr;
    for(uint64_t i = 0; i < x_data.size(); i++) {
        results_table_max << x_data[i].first << x_data[i].second;
        file << x_data[i].first << ","  << x_data[i].second;
        for (auto const& x : m) {
            auto y_data = std::get<2>(x);
            results_table_max << std::get<2>(y_data[i]) << std::get<3>(y_data[i]);
            file << "," << std::get<2>(y_data[i]) << "," << std::get<3>(y_data[i]);
        }
        results_table_max << fort::endr;
        file << "\n";
    }
    file.close();

    // table format
    results_table_max.row(0).set_cell_text_align(fort::text_align::center);
    std::cout << results_table_max.to_string() << std::endl;

    // check_philox_vs_simd();  // NOTE: must set UNSHUFFLE to 1 for generators to match exactly
}
