#include "Philox.h"
#include "MT19937.h"
#include "xoshiro256starstar.h"
#include "PhiloxSIMD.h"
#include "PCG.h"
#include <iostream>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>
#include <limits>
#include <thread>
#include <vector>
#include <string>
#include <cstring>

constexpr int MAX_THREADS = 1024;
constexpr int TRIALS = 3;
constexpr float POW_2_32_INV = 1.0f / std::numeric_limits<uint32_t>::max();

typedef std::chrono::time_point<std::chrono::high_resolution_clock> hres_t;
typedef std::pair<hres_t, hres_t> time_pair_t;

/**
 * Benchmark function that takes in a lambda and instruments it with timing
 * utilities.
 */
template <typename func_t>
static std::tuple<double, double, double, double> benchmark(std::string name, uint64_t loop_count, const func_t &f, uint64_t num_threads)
{
    std::cout << std::endl;
    std::cout << "Benchmarking: " << name << std::endl;

    // measures the best average time a thread took to execute a benchmark
    double best_avg = std::numeric_limits<double>::infinity();
    // measures the best difference between the time when the first thread started executing and the time
    // when the last thread stopped execution and joined.
    double best_max = best_avg;
    // measures the worst average time a thread took to execute a benchmark
    double worst_avg = 0.0;
    // measures the worst difference between the time when the first thread started executing and the time
    // when the last thread stopped execution and joined.
    double worst_max = 0.0;

    // collects all the start and end times of each thread
    std::vector<time_pair_t> times(num_threads);

    // defines a lambda that instruments a benchmark function with time
    auto timed_func = [&](int thread_idx) {
        times[thread_idx].first = std::chrono::high_resolution_clock::now();
        f(thread_idx);
        times[thread_idx].second = std::chrono::high_resolution_clock::now();
    };

    // run the benchmark for multiple trials
    for (int trial = 0; trial < TRIALS; trial++)
    {
        std::vector<std::thread> threads;

        for (uint64_t i = 0; i < num_threads; ++i)
        {
            threads.push_back(std::thread(timed_func, i));
        }

        for (auto &t : threads)
        {
            t.join();
        }

        // calculate best_avg and best_max
        double times_sum = 0;
        hres_t tmin = std::chrono::high_resolution_clock::now();
        hres_t tmax = hres_t();
        for (auto const &t : times)
        {
            std::chrono::duration<double> diff = t.second - t.first;

            if (tmin > t.first)
            {
                tmin = t.first;
            }
            if (tmax < t.second)
            {
                tmax = t.second;
            }
            times_sum += diff.count();
        }
        best_avg = std::min(best_avg, times_sum / num_threads);
        best_max = std::min(best_max, std::chrono::duration<double>(tmax - tmin).count());
        worst_avg = std::max(worst_avg, times_sum / num_threads);
        worst_max = std::max(worst_max, std::chrono::duration<double>(tmax - tmin).count());
    }
    std::cout << "To get " << loop_count << " " << name << " randoms, with "
              << num_threads << " thread(s), best average time per thread = " << best_avg << " (s),"
              << " best time it took to finish execution = " << best_max << " (s)"
              << " worst average time per thread = " << worst_avg << " (s),"
              << " worst time it took to finish execution = " << worst_max << " (s)"
              << std::endl;
    return std::make_tuple(best_avg, worst_avg, best_max, worst_max);
}

void check_philox_vs_simd()
{
    at::philox_engine philox(0, 0, 0);
    at::philox_simd_engine philox_simd(0, 0, 0);
    bool ok = true;
    for (int i = 0; i < 1000; i++)
    {
        uint32_t expected = philox();
        uint32_t actual = philox_simd();
        if (expected != actual)
        {
            printf("philox differs from philox_simd at %d (%08x vs %08x)\n", i, expected, actual);
            ok = false;
            break;
        }
    }
    if (ok)
    {
        printf("OK\n");
    }
}

std::tuple<double, double, double, double> philox_global_instance(std::string name, uint64_t loop_count = 134217728UL, uint64_t num_threads = 1)
{
    std::vector<uint32_t> y(num_threads, 0);
    at::philox_engine gen1(0, 0, 0);
    std::mutex mutex;
    uint64_t step = 128 / 32;
    auto bench = benchmark(name, loop_count, [&](uint64_t thread_idx) {
        uint32_t local = 0;
        at::detail::Array<uint32_t, 4> z;
        std::lock_guard<std::mutex> lock(mutex);
        for (uint64_t i = 0; i < loop_count / num_threads; i += step)
        {
            z = gen1.next();

            local += z[0];
            local += z[1];
            local += z[2];
            local += z[3];
        }

        y[thread_idx] = local;
    },
                           num_threads);
    uint32_t x = std::accumulate(y.begin(), y.end(), 0);
    std::cout << "Accumulated Y value is " << x << std::endl;
    return bench;
}

std::tuple<double, double, double, double> philox_thread_local_instance(std::string name, uint64_t loop_count = 134217728UL, uint64_t num_threads = 1)
{
    std::vector<uint32_t> y(num_threads, 0);
    uint64_t step = 128 / 32;
    std::vector<at::philox_engine> engines;
    for (uint64_t i = 0; i < num_threads; ++i) {
        engines.emplace_back(0, i, 0);
    }
    auto bench = benchmark(name, loop_count, [&](uint64_t thread_idx) {
        uint32_t local = 0;
        at::detail::Array<uint32_t, 4> z;
        auto &gen1 = engines[thread_idx];
        for (uint64_t i = 0; i < loop_count / num_threads; i += step)
        {
            z = gen1.next();
            local += z[0];
            local += z[1];
            local += z[2];
            local += z[3];
        }
        y[thread_idx] = local;
    },
                           num_threads);
    uint32_t x = std::accumulate(y.begin(), y.end(), 0);
    std::cout << "Accumulated Y value is " << x << std::endl;
    return bench;
}

std::tuple<double, double, double, double> philox_simd_global_instance(std::string name, uint64_t loop_count = 134217728UL, uint64_t num_threads = 1)
{
    at::philox_simd_engine gen(0, 0, 0);
    std::mutex mutex;
    uint64_t step = 1024 / 32;

    __m256i y[MAX_THREADS];
    memset(y, 0, sizeof(y[0]) * MAX_THREADS);
    auto bench = benchmark(name, loop_count, [&](uint64_t thread_idx) {
        __m256i a, b, c, d;
        __m256i v = _mm256_set1_epi32(0);
        std::lock_guard<std::mutex> lock(mutex);
        for (uint64_t i = 0; i < loop_count / num_threads; i += step)
        {
            gen.next32(a, b, c, d);
            v = _mm256_add_epi32(v, a);
            v = _mm256_add_epi32(v, b);
            v = _mm256_add_epi32(v, c);
            v = _mm256_add_epi32(v, d);
        }
        y[thread_idx] = v;
    },
                           num_threads);

    uint32_t x = 0;
    uint32_t values[8];
    for (uint64_t j = 0; j < num_threads; ++j)
    {
        _mm256_storeu_si256((__m256i *)values, y[j]);
        for (int i = 0; i < 8; i++)
        {
            x += values[i];
        }
    }

    std::cout << "Accumulated Y value is " << x << std::endl;
    return bench;
}

std::tuple<double, double, double, double> philox_simd_thread_local_instance(std::string name, uint64_t loop_count = 134217728UL, uint64_t num_threads = 1)
{
    uint64_t step = 1024 / 32;
    __m256i y[MAX_THREADS];
    memset(y, 0, sizeof(y[0]) * MAX_THREADS);
    std::vector<at::philox_simd_engine> engines;
    for (uint64_t i = 0; i < num_threads; ++i)
    {
        engines.emplace_back(0, i, 0);
    }
    auto bench = benchmark(name, loop_count, [&](uint64_t thread_idx) {
        __m256i a, b, c, d;
        __m256i v = _mm256_set1_epi32(0);
        auto &gen = engines[thread_idx];
        for (uint64_t i = 0; i < loop_count / num_threads; i += step)
        {
           
            gen.next32(a, b, c, d);
            v = _mm256_add_epi32(v, a);
            v = _mm256_add_epi32(v, b);
            v = _mm256_add_epi32(v, c);
            v = _mm256_add_epi32(v, d);
        }
        y[thread_idx] = v;
    },
                           num_threads);
    uint32_t x = 0;
    uint32_t values[8];
    for (uint64_t j = 0; j < num_threads; ++j)
    {
        _mm256_storeu_si256((__m256i *)values, y[j]);
        for (int i = 0; i < 8; i++)
        {
            x += values[i];
        }
    }

    std::cout << "Accumulated Y value is " << x << std::endl;
    return bench;
}

std::tuple<double, double, double, double> xoshiro256(std::string name, uint64_t loop_count = 134217728UL, uint64_t num_threads = 1)
{
    std::vector<uint64_t> y(num_threads, 0);
    xoshiro256starstar_engine gen1(0);
    std::mutex mutex;
    uint64_t step = 64 / 32;
    auto bench = benchmark(name, loop_count, [&](uint64_t thread_idx) {
        uint64_t z = 0;
        std::lock_guard<std::mutex> lock(mutex);
        for (uint64_t i = 0; i < loop_count / num_threads; i += step)
        {
            z += gen1.next();
        }
        y[thread_idx] = z;
    },
                           num_threads);
    uint64_t x = std::accumulate(y.begin(), y.end(), 0);
    std::cout << "Accumulated Y value is " << x << std::endl;
    return bench;
}

std::tuple<double, double, double, double> at_pcg(std::string name, uint64_t loop_count = 134217728UL, uint64_t num_threads = 1)
{
    std::vector<uint64_t> y(num_threads, 0);
    at::pcg gen1;
    std::mutex mutex;
    uint64_t step = 64 / 32;
    auto bench = benchmark(name, loop_count, [&](uint64_t thread_idx) {
        uint64_t z = 0;
        std::lock_guard<std::mutex> lock(mutex);
        for (uint64_t i = 0; i < loop_count / num_threads; i += step)
        {
            z += gen1();
        }
        y[thread_idx] = z;
    },
                           num_threads);
    uint64_t x = std::accumulate(y.begin(), y.end(), 0);
    std::cout << "Accumulated Y value is " << x << std::endl;
    return bench;
}

std::tuple<double, double, double, double> at_mt19937(std::string name, uint64_t loop_count = 134217728UL, uint64_t num_threads = 1)
{
    std::vector<uint32_t> y(num_threads, 0);
    at::mt19937 gen1;
    std::mutex mutex;
    uint64_t step = 32 / 32;
    auto bench = benchmark(name, loop_count, [&](uint64_t thread_idx) {
        uint32_t z = 0;
        std::lock_guard<std::mutex> lock(mutex);
        for (uint64_t i = 0; i < loop_count / num_threads; i += step)
        {
            z += gen1();
        }
        y[thread_idx] = z;
    },
                           num_threads);
    uint32_t x = std::accumulate(y.begin(), y.end(), 0);
    std::cout << "Accumulated Y value is " << x << std::endl;
    return bench;
}

std::tuple<double, double, double, double> std_mt19937(std::string name, uint64_t loop_count = 134217728UL, uint64_t num_threads = 1)
{
    std::vector<uint32_t> y(num_threads, 0);
    std::mt19937 gen1;
    std::mutex mutex;
    uint64_t step = 32 / 32;
    auto bench = benchmark(name, loop_count, [&](uint64_t thread_idx) {
        uint32_t z = 0;
        std::lock_guard<std::mutex> lock(mutex);
        for (uint64_t i = 0; i < loop_count / num_threads; i += step)
        {
            z = gen1();
        }
        y[thread_idx] = z;
    },
                           num_threads);
    uint32_t x = std::accumulate(y.begin(), y.end(), 0);
    std::cout << "Accumulated Y value is " << x << std::endl;
    return bench;
}
