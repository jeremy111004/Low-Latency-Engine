#pragma once
#include <cstdint>

struct Order {
  Order *next;
  Order *prev;
  uint32_t id;
  uint32_t price;
  uint32_t quantity;
  char type;
  bool enabled;

  // 'A' for Ask, 'B' for Bid
  // note: id > other.id is a seniority comparison, as ID1
  // > ID2 means that ID2 is older.
  bool operator>(const Order &other) const {
    if (price != other.price)
      return price > other.price;
    return id > other.id;
  }
  bool operator<(const Order &other) const {
    if (price != other.price)
      return price < other.price;
    return id < other.id;
  }
};

struct matchResult {
  uint64_t id1;
  uint64_t id2;
  uint32_t quantity;
  uint32_t price;
};
