# Random Number Engine Benchmark

`benchmark.cpp` benchmarks `Philox.h`, `MT19937.h`, `PCG.h` and `std::mt19937`.

Build and run with the following instructions:
```
g++ --std=c++11 -c -O3 -o funcs.o functions.cpp
g++ --std=c++11 -c -O3 -o main.o benchmark.cpp
g++ -o bench main.o funcs.o
./bench

```

# Results:
Benchmarked on DGX systems with Intel Xeons, G++ 5.4.0.
#### without any optimizations
```
Time to get 100000000 philox randoms with at::uniform_real_distribution = 10.0938s
X value is 0.128836
Time to get 100000000 at::mt19937 randoms with at::uniform_real_distribution = 2.29676s
X value is 0.365931
Time to get 100000000 std::mt19937 randoms with std::uniform_real_distribution = 13.9122s
X value is 0.365931
Time to get 100000000 std::mt19937 randoms with at::uniform_real_distribution = 1.92397s
X value is 0.365931
```

#### with -O2
```
Time to get 100000000 philox randoms with at::uniform_real_distribution = 0.462759s
X value is 0.128836
Time to get 100000000 at::mt19937 randoms with at::uniform_real_distribution = 0.39628s
X value is 0.365931
Time to get 100000000 std::mt19937 randoms with std::uniform_real_distribution = 0.352087s
X value is 0.365931
Time to get 100000000 std::mt19937 randoms with at::uniform_real_distribution = 0.419454s
X value is 0.365931
```
#### with -O3
```
Time to get 100000000 philox randoms with at::uniform_real_distribution = 0.479021s
X value is 0.128836
Time to get 100000000 at::mt19937 randoms with at::uniform_real_distribution = 0.320545s
X value is 0.365931
Time to get 100000000 std::mt19937 randoms with std::uniform_real_distribution = 0.393593s
X value is 0.365931
Time to get 100000000 std::mt19937 randoms with at::uniform_real_distribution = 0.461688s
X value is 0.365931
```
