#include "OrderBook.hpp"
#include <iomanip>
#include <iostream>

void NaiveLOB::addOrder(Order ord) {
  if (ord.side == Side::Buy) {
    matching(ord, priceMapAsk, [](uint32_t buyPrice, uint32_t askPrice) {
      return buyPrice >= askPrice;
    });
    if (ord.quantity > 0) {
      auto &level = priceMapBid[ord.price];
      level.totalVolume += ord.quantity;
      level.orders.push_back(ord);
      orderMap[ord.id] = {ord.price, ord.side, std::prev(level.orders.end())};
    }
  } else {
    matching(ord, priceMapBid, [](uint32_t sellPrice, uint32_t bidPrice) {
      return sellPrice <= bidPrice;
    });
    if (ord.quantity > 0) {
      auto &level = priceMapAsk[ord.price];
      level.totalVolume += ord.quantity;
      level.orders.push_back(ord);
      orderMap[ord.id] = {ord.price, ord.side, std::prev(level.orders.end())};
    }
  }
}

void NaiveLOB::cancelOrder(uint64_t id) {
  auto it = orderMap.find(id);
  if (it == orderMap.end())
    return;

  const OrderEntry &entry = it->second;

  if (entry.side == Side::Buy) {
    auto mapIt = priceMapBid.find(entry.price);
    if (mapIt != priceMapBid.end()) {
      mapIt->second.totalVolume -= entry.it->quantity;
      mapIt->second.orders.erase(entry.it);
      if (mapIt->second.orders.empty())
        priceMapBid.erase(mapIt);
    }
  } else {
    auto mapIt = priceMapAsk.find(entry.price);
    if (mapIt != priceMapAsk.end()) {
      mapIt->second.totalVolume -= entry.it->quantity;
      mapIt->second.orders.erase(entry.it);
      if (mapIt->second.orders.empty())
        priceMapAsk.erase(mapIt);
    }
  }
  orderMap.erase(it);
}

uint64_t NaiveLOB::editOrder(uint64_t id, uint32_t newPrice,
                             uint32_t newQuantity) {
  auto it = orderMap.find(id);
  if (it == orderMap.end())
    return 0;

  Order updatedOrder = *(it->second.it);
  cancelOrder(id);

  updatedOrder.price = newPrice;
  updatedOrder.quantity = newQuantity;

  addOrder(updatedOrder);
  return updatedOrder.id;
}

void NaiveLOB::reset() {
  priceMapAsk.clear();
  priceMapBid.clear();
  orderMap.clear();
}

void NaiveLOB::seeBidRank() const {
  for (const auto &[price, level] : priceMapBid) {
    std::cout << "Price: " << price << " | Volume: " << level.totalVolume
              << std::endl;
  }
}

void NaiveLOB::seeAskRank() const {
  for (const auto &[price, level] : priceMapAsk) {
    std::cout << "Price: " << price << " | Volume: " << level.totalVolume
              << std::endl;
  }
}
