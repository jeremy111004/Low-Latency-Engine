# Low-Latency Matching Engine (C++20)

Personal project I started to learn high-performance C++. I keep improving it
as a way to really understand low-latency programming and everything that comes with it.

It started as a simple std::map / std::list matching engine. I then rewrote it
step by step around the cache: struct-of-arrays layout, bitmaps to find the
best price in O(1), and no allocations on the hot path.

## Results

Single core, 20,000 synthetic orders, Google Benchmark:

| Version  | Design                                                  | Throughput      |
|----------|---------------------------------------------------------|-----------------|
| Baseline | std::map + std::list + std::unordered_map               | ~31M orders/s   |
| v2       | intrusive lists, arena allocator, price bitmaps         | ~101M orders/s  |
| v3       | struct of arrays, ring buffer per price level           | ~125M orders/s  |

v3 is about 4x the first version, about 8 ns per order on average.

I'm currently improving the parser to make it more realistic and to build a
full system with different layers working together (parser -> lock-free SPSC
queue -> matching engine).

## Build

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    cd build
    ./bench_v3
