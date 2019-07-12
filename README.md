# Random Number Engine Benchmark

`benchmark.cpp` benchmarks `Philox.h`, `PhiloxSIMD.h` `xoshiro256starstar.h`, `PCG.h` and `std::mt19937`

Build and run with the following instructions:
```
g++ --std=c++11 -O3 -pthread -march=native functions.cpp benchmark.cpp -lm -o bench
./bench
./bench --num-threads 5
```

# Usage:
```
Usage: ./bench [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -l,--loop-count UINT        Number of randoms to produce per thread
  -t,--num-threads INT        Number of threads
```

# Results:
Benchmarked on Intel(R) Xeon(R) CPU E5-2698 v4 @ 2.20GHz with GCC 7.3.0
#### with -O3
```
Time to get 100000000 philox randoms = 0.271853s
Y value is 3011083429
Time to get 100000000 philox_simd randoms = 0.117607s
Y value is 3011083429
Time to get 100000000 xoshiro256** randoms = 0.0561525s
Y value is 2135289733523560858
Time to get 100000000 pcg64 randoms = 0.0990407s
Y value is 18182947979635078005
Time to get 100000000 at::mt19937 randoms = 0.235999s
Y value is 3753120473
Time to get 100000000 std::mt19937 randoms = 0.195286s
Y value is 3753120473
```
