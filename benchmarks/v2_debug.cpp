#include <iostream>
#include "OrderBook2.hpp"

int main() {
    LOB lob;

    // 1. Manual Orders (These must exist for the duration of the test)
    Order b1{ 1, nullptr, nullptr, 99, 100, 'B', true };
    Order a1{ 3, nullptr, nullptr, 101, 80, 'A', true };

    // 2. Put the orders INTO the LOB object.
    // We don't redeclare PriceList or the arrays here. 
    // We use the ones ALREADY inside 'lob'.
    lob.priceMapBid[99] = { &b1, &b1 }; 
    lob.priceMapAsk[101] = { &a1, &a1 };

    // 3. Run the finder
    // We capture the result in a variable so we can actually see it.
    int bestAsk = lob.indexBestPriceFinder<LOB::Side::Buy>();
    int bestBid = lob.indexBestPriceFinder<LOB::Side::Sell>();

    // 4. Print results
    std::cout << "Best Ask found at index: " << bestAsk << " (Expected 101)" << std::endl;
    std::cout << "Best Bid found at index: " << bestBid << " (Expected 99)" << std::endl;

    return 0;
}
