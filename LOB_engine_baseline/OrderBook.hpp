#pragma once

#include "Order.hpp"
#include <algorithm>
#include <cstdint>
#include <list>
#include <map>
#include <unordered_map>

struct NaivePriceLevel {
  std::list<Order> orders;
  uint64_t totalVolume = 0;

  size_t numberOfOrders() const { return orders.size(); }
};

class NaiveLOB {
public:
  // Core API
  void addOrder(Order ord);
  void cancelOrder(uint64_t id);
  uint64_t editOrder(uint64_t id, uint32_t newPrice, uint32_t newQuantity);
  void reset();

  // Utilities
  void seeBidRank() const;
  void seeAskRank() const;

  /**
   * Template definition must reside in the header to allow instantiation.
   */
  template <typename OppositeMap, typename Tcompare>
  void matching(Order &ord, OppositeMap &oppositeMap, Tcompare tcompare) {
    auto it = oppositeMap.begin();
    while (it != oppositeMap.end() && ord.quantity > 0 &&
           tcompare(ord.price, it->first)) {
      auto &level = it->second;
      auto listIt = level.orders.begin();

      while (listIt != level.orders.end() && ord.quantity > 0) {
        uint32_t matchedQty = std::min(ord.quantity, listIt->quantity);
        ord.quantity -= matchedQty;
        listIt->quantity -= matchedQty;
        level.totalVolume -= matchedQty;

        if (listIt->quantity == 0) {
          orderMap.erase(listIt->id);
          listIt = level.orders.erase(listIt);
        } else {
          ++listIt;
        }
      }

      if (level.orders.empty()) {
        it = oppositeMap.erase(it);
      } else {
        ++it;
      }
    }
  }

private:
  std::map<uint32_t, NaivePriceLevel> priceMapAsk;
  std::map<uint32_t, NaivePriceLevel, std::greater<uint32_t>> priceMapBid;

  struct OrderEntry {
    uint32_t price;
    Side side;
    std::list<Order>::iterator it;
  };
  std::unordered_map<uint64_t, OrderEntry> orderMap;
};
