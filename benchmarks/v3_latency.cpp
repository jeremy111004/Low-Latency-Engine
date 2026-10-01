// v3_latency.cpp
// Per-order latency distribution for the v3 (SoA) order book.
// Same seeded workload as v3_main.cpp, so the numbers are comparable.
// Standalone executable: does NOT use Google Benchmark.
// Timing uses the CPU timestamp counter (rdtsc) directly, x86-64 only,
// because the OS clock can cost ~1 us per read on some systems (VMs, HPET).

#include "OrderBook2.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>
#include <x86intrin.h>

using Clock = std::chrono::steady_clock;

// lfence stops the CPU from reordering work across the timestamp read.
static inline uint64_t tsc() {
    _mm_lfence();
    uint64_t t = __rdtsc();
    _mm_lfence();
    return t;
}

// How many TSC ticks per nanosecond (measured once, over ~200 ms).
static double calibrateTicksPerNs() {
    auto c0 = Clock::now();
    uint64_t t0 = tsc();
    while (Clock::now() - c0 < std::chrono::milliseconds(200)) {}
    uint64_t t1 = tsc();
    auto c1 = Clock::now();
    double ns = std::chrono::duration_cast<std::chrono::nanoseconds>(c1 - c0).count();
    return (t1 - t0) / ns;
}

struct BenchOrder {
    uint32_t id;
    uint16_t price;
    uint16_t quantity;
    Side side;
};

static std::vector<BenchOrder> makeWorkload(LOB& lob, int numberOrders) {
    std::mt19937 gen(42);  // same seed as v3_main.cpp
    std::normal_distribution<double> distribP(100.0, 2.0);
    std::uniform_int_distribution<> distribQ(1, 50);

    std::vector<BenchOrder> data;
    data.reserve(numberOrders * 2);
    for (int c = 0; c < numberOrders; c++) {
        auto idAsk = static_cast<uint32_t>(lob.generateID());
        auto idBid = static_cast<uint32_t>(lob.generateID());
        data.push_back({idAsk, static_cast<uint16_t>(std::round(distribP(gen))),
                        static_cast<uint16_t>(distribQ(gen)), Side::Sell});
        data.push_back({idBid, static_cast<uint16_t>(std::round(distribP(gen))),
                        static_cast<uint16_t>(distribQ(gen)), Side::Buy});
    }
    return data;
}

// Cost of the two timestamp reads alone (in ticks), so it can be subtracted.
static int64_t measureClockOverhead() {
    constexpr int N = 100000;
    std::vector<int64_t> v(N);
    for (int i = 0; i < N; i++) {
        uint64_t t0 = tsc();
        uint64_t t1 = tsc();
        v[i] = static_cast<int64_t>(t1 - t0);
    }
    std::sort(v.begin(), v.end());
    return v[N / 2];  // median overhead
}

static int64_t pct(const std::vector<int64_t>& sorted, double p) {
    auto idx = static_cast<size_t>(p * (sorted.size() - 1));
    return sorted[idx];
}

int main() {
    static LOB lob;  // static: the SoA arrays are large, keep them off the stack
    constexpr int numberOrders = 10000;  // -> 20,000 orders per run
    constexpr int warmupRuns = 20;
    constexpr int measuredRuns = 200;

    auto workload = makeWorkload(lob, numberOrders);

    // Warm-up: fills caches and branch predictors, not recorded.
    for (int r = 0; r < warmupRuns; r++) {
        lob.reset();
        for (const auto& o : workload)
            lob.processOrder(o.quantity, o.id, o.price, o.side);
    }

    std::vector<int64_t> lat;
    lat.reserve(static_cast<size_t>(measuredRuns) * workload.size());

    for (int r = 0; r < measuredRuns; r++) {
        lob.reset();  // not timed
        for (const auto& o : workload) {
            uint64_t t0 = tsc();
            lob.processOrder(o.quantity, o.id, o.price, o.side);
            uint64_t t1 = tsc();
            lat.push_back(static_cast<int64_t>(t1 - t0));  // ticks
        }
    }

    const double ticksPerNs = calibrateTicksPerNs();
    const int64_t overheadTicks = measureClockOverhead();

    // Convert ticks -> ns
    for (auto& x : lat) x = static_cast<int64_t>(std::llround(x / ticksPerNs));
    const int64_t overhead = static_cast<int64_t>(std::llround(overheadTicks / ticksPerNs));
    std::sort(lat.begin(), lat.end());

    double mean = 0;
    for (auto x : lat) mean += x;
    mean /= lat.size();

    std::printf("TSC frequency      : %.3f GHz\n", ticksPerNs);
    std::printf("Samples            : %zu orders\n", lat.size());
    std::printf("Clock overhead     : %lld ns (median, subtract from figures below)\n",
                (long long)overhead);
    std::printf("Raw latency (ns)   : mean %.1f | p50 %lld | p90 %lld | p99 %lld | "
                "p99.9 %lld | max %lld\n",
                mean, (long long)pct(lat, 0.50), (long long)pct(lat, 0.90),
                (long long)pct(lat, 0.99), (long long)pct(lat, 0.999),
                (long long)lat.back());
    std::printf("Net of overhead    : p50 %lld | p99 %lld | p99.9 %lld\n",
                (long long)std::max<int64_t>(0, pct(lat, 0.50) - overhead),
                (long long)std::max<int64_t>(0, pct(lat, 0.99) - overhead),
                (long long)std::max<int64_t>(0, pct(lat, 0.999) - overhead));
    return 0;
}
