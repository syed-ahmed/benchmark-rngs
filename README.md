# Benchmark for PyTorch

`benchmark.cpp` benchmarks `Philox.h`, `PhiloxSIMD.h` `xoshiro256starstar.h`, `pcg64.h` and `std::mt19937`.

Build and run with the following instructions:
```
g++ --std=c++11 -O3 -march=native functions.cpp benchmark.cpp -lm -o bench
./bench

```

# Results:
Benchmarked on Intel(R) Xeon(R) CPU E5-2698 v4 @ 2.20GHz with GCC 7.3.0
#### with -O3
```
Time to get 100000000 philox randoms = 0.269777s
Y value is 3011083429
Time to get 100000000 philox_simd randoms = 0.117817s
Y value is 3011083427
Time to get 100000000 xoshiro256** randoms = 0.0565056s
Y value is 2135289733523560858
Time to get 100000000 pcg64 randoms = 0.099247s
Y value is 18182947979635078005
Time to get 100000000 at::mt19937 randoms = 0.233224s
Y value is 3753120473
Time to get 100000000 std::mt19937 randoms = 0.190974s
Y value is 3753120473
```
