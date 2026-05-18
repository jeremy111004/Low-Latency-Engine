#include "OrderBook.hpp"
#include <benchmark/benchmark.h>
#include <cstdint>
#include <random>
#include <vector>

Order OrdersGenerator(uint32_t price, Side side, uint64_t id) {
  int min = price - 4;
  int max = price + 4;

  static std::random_device rd;
  static std::mt19937 gen(rd());

  std::uniform_int_distribution<> distribP(min, max);
  std::uniform_int_distribution<> distribQ(1, 50);

  return Order{id, static_cast<uint32_t>(distribP(gen)),
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
