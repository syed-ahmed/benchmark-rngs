# Random Number Engine Benchmark

`benchmark.cpp` benchmarks `Philox.h`, `PhiloxSIMD.h` `xoshiro256starstar.h`, `PCG.h` and `std::mt19937`

Build and run with the following instructions:
```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make VERBOSE=1
./bench
```

# Usage:
```
Random Number Engine Benchmark
Usage: ./bench [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -r,--min-num-randoms UINT   Minimum number of randoms to produce
  -t,--min-num-threads INT    Minimum number of threads to use
  -e,--engine UINT ...        Selects specific tests to run by supplying an int. Runs with all tests if none provided.
                              Available tests are:
                              {0: philox (global)}
                              {1: philox (thread local)}
                              {2: philox_simd (global)}
                              {3: philox_simd (thread local)}
                              {4: xoshiro256**}
                              {5: pcg64}
                              {6: at::mt19937}
                              {7: std::mt19937}
                              {8: at::mt19937 (chunking)}
                              {9: pcg64 (chunking)}
                              {10: std::mt19937 (chunking)}
                              {11: xoshiro256** (chunking)}
                              {12: philox (global) (chunking)}
                              {13: philox_simd (global) (chunking)}
  -x,--num-x-data-points INT  Bins of x data points to produce, where x is either threads or number of randoms
[Option Group: benchmark_type]
  Decides if the independent variable is number of threads or number of randoms 
  [At most 1 of the following options are allowed]
  Options:
    -a,--benchmark-threads      Run full benchmark with increasing number of threads
    -b,--benchmark-num-randoms  Run full benchmark with increasing number of randoms
```
