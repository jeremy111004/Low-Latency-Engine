#pragma once

#include <cstdint>

enum class Side : uint8_t { Buy, Sell };

struct Order {
  uint64_t id;
  uint32_t price;
  uint32_t quantity;
  Side side;

  Order() = default;

  Order(uint64_t id, uint32_t price, uint32_t quantity, Side side)
      : id(id), price(price), quantity(quantity), side(side) {}
};

struct matchResult {
  uint64_t makerId;
  uint64_t takerId;
  uint32_t price;
  uint32_t quantity;
};
