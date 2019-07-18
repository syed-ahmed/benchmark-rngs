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

# Running the benchmark:
### Step 1: Adjust cpu settings
#### Turn off turbo-boost (if available) using the following script (https://askubuntu.com/questions/619875/disabling-intel-turbo-boost-in-ubuntu):
```
#!/bin/bash

if [[ -z $(which rdmsr) ]]; then
    echo "msr-tools is not installed. Run 'sudo apt-get install msr-tools' to install it." >&2
    exit 1
fi

if [[ ! -z $1 && $1 != "enable" && $1 != "disable" ]]; then
    echo "Invalid argument: $1" >&2
    echo ""
    echo "Usage: $(basename $0) [disable|enable]"
    exit 1
fi

cores=$(cat /proc/cpuinfo | grep processor | awk '{print $3}')
for core in $cores; do
    if [[ $1 == "disable" ]]; then
        sudo wrmsr -p${core} 0x1a0 0x4000850089
    fi
    if [[ $1 == "enable" ]]; then
        sudo wrmsr -p${core} 0x1a0 0x850089
    fi
    state=$(sudo rdmsr -p${core} 0x1a0 -f 38:38)
    if [[ $state -eq 1 ]]; then
        echo "core ${core}: disabled"
    else
        echo "core ${core}: enabled"
    fi
done
```
#### Adjust NUMA if needed (not required if your CPUs are in one socket)
https://developer.download.nvidia.com/video/gputechconf/gtc/2019/presentation/s9956-best-practices-when-benchmarking-cuda-applications_V2.pdf
#### Set CPU frequency to performance mode:
```
sudo cpupower frequency-set -g performance
```
#### Use i7z to monitor benchmark run
```
sudo i7z
```
### Step 2: Run benchmark
```
# benchmarks with varying number of threads, get 10 
./bench -a
# benchmarks with varying number of randoms
./bench -b
# benchmark a subset of engines
./bench -e 0 1 -a
```
### Step 3: Copy results to result folder
```
# from build folder
mv result-increasing-* ../results/
cd ../results/
jupyter notebook # run plot-results.ipynb
```