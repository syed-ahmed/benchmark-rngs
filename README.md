# Benchmark for PyTorch [PR#16604](https://github.com/pytorch/pytorch/pull/16604)

`benchmark.cpp` benchmarks `Philox.h`, `MT19937.h` and `std::mt19937`.

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

# Conclusion:
- Without any optimizations, **std::mt19937** randoms with **at::uniform_real_distribution** is the fastest.
- With O2, **std::mt19937** randoms with **std::uniform_real_distribution** is the fastest.
- With O3, **at::mt19937** randoms with **at::uniform_real_distribution** is the fastest.
- PyTorch builds with -O2, so it would just make sense to go with std. However, std::uniform_real_distribution
suffers with numbers not being in the range [0,1) - http://open-std.org/JTC1/SC22/WG21/docs/lwg-active.html#2524.
That limits us to use our custom at::uniform_real_distribution. Hence, we are going with at::mt19937+at::uniform_real_distribution. Morevoer, std::mt19937 can't be seeded properly (std::seed_seq is biased, and using a single 32-bit is not recommended: http://www.pcg-random.org/posts/cpp-seeding-surprises.html). std::mt19937 probably should have a constructor that can take a user supplied array, or std::seed_seq might need to be looked at as suggested by the article.
