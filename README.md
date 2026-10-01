# Low-Latency Order Book (C++20)

Personal project I started to learn high-performance C++. I keep improving it
as a way to really understand low-latency programming and everything coming with it.

It started as a simple std::map / std::list matching engine. I then rewrote it
step by step around the cache: struct-of-arrays layout, bitmaps to find the
best price in O(1), and no allocations on the hot path. A CSV parser feeds the
engine through a lock-free SPSC queue.

Current result: 115M+ orders/sec (about 4x the first version), ~10 ns median
latency per order, single core.

I'm currently improving the implementation of the parser to make it more realistic 
and to have a full system with different layers working together .

Build:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    ./build/bench_v3
