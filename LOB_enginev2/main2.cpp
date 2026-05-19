#include "OrderBook2.hpp"
#include <benchmark/benchmark.h>
#include <benchmark/benchmark_api.h>
#include <benchmark/registration.h>
#include <benchmark/state.h>
#include <benchmark/utils.h>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

// The Generator is kept here because the LOB should not have access
// or see how fake orders are created.
Order OrdersGenerator(uint32_t price, char type, uint64_t id, std::mt19937& gen) {
  int min = price - 4;
  int max = price + 4;

  

  std::uniform_int_distribution<> distribP(min, max);
  std::uniform_int_distribution<> distribQ(1, 50);

  // C++20 Designated Initializers: Clear, safe, and warning-free
  return Order{.next = nullptr,
               .prev = nullptr,
	       .id = static_cast<uint32_t>(id),
               .price = static_cast<uint32_t>(distribP(gen)),
               .quantity = static_cast<uint32_t>(distribQ(gen)),
               .type = type,
               .enabled = true};
}

static void LOB_Continuous(benchmark::State &state) {
  static LOB lob; // Instantiate ONCE outside the loop
  const int numberOrders = 10000;
  std::vector<Order> SyntheticData;
  SyntheticData.reserve(numberOrders * 2);
  uint32_t seed = 42; 
  std::mt19937 gen(seed);
 

  // Pre-generate data
  for (int c = 0; c < numberOrders; c++) {
    auto IDask = lob.generateID();
    auto IDbid = lob.generateID();
    SyntheticData.push_back(OrdersGenerator(100, 'A', IDask,gen));
    SyntheticData.push_back(OrdersGenerator(100, 'B', IDbid,gen));
  }

  const int totalSize = SyntheticData.size();

  for (auto _ : state) {
    state.PauseTiming();
    lob.reset(); // Dedicated reset is faster than reconstruction
    state.ResumeTiming();

    for (int i = 0; i < totalSize; i++) {
      Order ordcopy = SyntheticData[i];
      lob.addOrder(ordcopy);
    }

    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(state.iterations() * totalSize);
}


BENCHMARK(LOB_Continuous)->Iterations(500)->Unit(benchmark::kNanosecond);
