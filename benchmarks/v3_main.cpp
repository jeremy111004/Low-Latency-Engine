#include <tuple>
#include <vector>
#include <random>
#include <cmath>
#include <benchmark/benchmark.h>
#include "OrderBook2.hpp"

std::tuple<uint32_t, uint16_t, uint16_t> OrdersGenerator(uint16_t price, char type, uint32_t id, std::mt19937& gen) {
    std::normal_distribution<double> distribP(static_cast<double>(price), 2.0);
    std::uniform_int_distribution<> distribQ(1, 50); 
    
    uint16_t generatedPrice = static_cast<uint16_t>(std::round(distribP(gen)));
    uint16_t generatedQty = static_cast<uint16_t>(distribQ(gen));
    
    return std::make_tuple(id, generatedPrice, generatedQty);
}

static void LOB_Continuous(benchmark::State &state) {
    static LOB lob; 
    const int numberOrders = 10000;
    
    struct BenchOrder {
        uint32_t id;
        uint16_t price;
        uint16_t quantity;
        Side side;
    };
    
    std::vector<BenchOrder> SyntheticData;
    SyntheticData.reserve(numberOrders * 2);

    uint32_t seed = 42; 
    std::mt19937 gen(seed);

    for (int c = 0; c < numberOrders; c++) {
        auto IDask = static_cast<uint32_t>(lob.generateID());
        auto IDbid = static_cast<uint32_t>(lob.generateID());
        
        auto OrderAsk = OrdersGenerator(100, 'A', IDask, gen);
        SyntheticData.push_back({
            std::get<0>(OrderAsk),
            std::get<1>(OrderAsk),
            std::get<2>(OrderAsk),
            Side::Sell
        });

        auto OrderBid = OrdersGenerator(100, 'B', IDbid, gen);
        SyntheticData.push_back({
            std::get<0>(OrderBid),
            std::get<1>(OrderBid),
            std::get<2>(OrderBid),
            Side::Buy
        });
    }

    const int totalSize = SyntheticData.size();

    for (auto _ : state) {
        state.PauseTiming();
        lob.reset(); 
        state.ResumeTiming();

        for (int i = 0; i < totalSize; i++) {
            const auto &ord = SyntheticData[i];

            lob.processOrder(ord.quantity, ord.id, ord.price, ord.side);
        }
    }
    state.SetItemsProcessed(state.iterations() * totalSize);
}
BENCHMARK(LOB_Continuous)->Iterations(500)->Unit(benchmark::kNanosecond);
