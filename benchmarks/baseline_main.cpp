#include "OrderBook.hpp"
#include <benchmark/benchmark.h>
#include <cstdint>
#include <random>
#include <vector>
#include <cmath>

Order OrdersGenerator(uint32_t price, Side side, uint64_t id) {
  static std::mt19937 gen(42);  // same seed as v3
  std::normal_distribution<double> distribP(static_cast<double>(price), 2.0);
  std::uniform_int_distribution<> distribQ(1, 50);

  return Order{id, static_cast<uint32_t>(std::round(distribP(gen))),
               static_cast<uint32_t>(distribQ(gen)), side};
}

static void LOB_Naive_Baseline(benchmark::State &state) {
  NaiveLOB lob;
  const int numberOrders = 10000;
  std::vector<Order> SyntheticData;

  uint64_t idCounter = 1;
  for (int c = 0; c < numberOrders; c++) {
    SyntheticData.push_back(OrdersGenerator(100, Side::Sell, idCounter++));
    SyntheticData.push_back(OrdersGenerator(100, Side::Buy, idCounter++));
  }

  const int totalSize = SyntheticData.size();

  for (auto _ : state) {
    state.PauseTiming();
    lob.reset();
    state.ResumeTiming();

    for (int i = 0; i < totalSize; i++) {
      lob.addOrder(SyntheticData[i]);
    }

    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(state.iterations() * totalSize);
}

BENCHMARK(LOB_Naive_Baseline);
