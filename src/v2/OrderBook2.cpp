#include "OrderBook2.hpp"
#include "Order2.hpp"
#include <atomic>
#include <cstdint>
#include <iostream>
#include <map>
#include <unordered_map>
#include <utility>
#include <cmath>

uint64_t LOB::generateID() {
	return NextID++; 
}

void LOB::reset() {
  std::fill(orderArray.begin(),orderArray.end(),orderEntry(nullptr,nullptr));
  matchedList.clear(); 
  auto itask = priceMapAsk.begin();
  auto itbid = priceMapBid.begin();
  while(itask != priceMapAsk.end() && itbid !=priceMapBid.end()){
	itask->nullify();
	itbid->nullify();
	itask++;
	itbid++;
  }
  NextID = 0;
  arena.reset();
}
void LOB::processOrder(Order& ord){
	if(ord.type == 'A'){
		LOB::matching<Side::Sell>(ord);
	}
	else{
		LOB::matching<Side::Buy>(ord);
	}
};

void LOB::addOrder(Order &ord) {
  processOrder(ord); 

  // if the order hasn't been fully matched (or not matched at all) we have to
  // actually save it:
  if (ord.quantity > 0) [[likely]] {
    // this line create an order directly in the pre allocated memory Arena
    Order *slot = arena.allocate<Order>();
    if (!slot) [[unlikely]] return;
    Order *order_ptr = new (slot) Order(ord);    
    bool isBuy = ord.type == 'B';

    auto &priceLevel =
        isBuy ? priceMapBid[ord.price] : priceMapAsk[ord.price];
    auto &bitIndexMap = isBuy? bitIndexBid : bitIndexAsk;

      bitIndexMap[ord.price/64] |= 1ULL << (ord.price%64); 
          // we link the current order with the last tail;
      order_ptr->prev = priceLevel.tail;
      priceLevel.tail->next = order_ptr;
      priceLevel.tail =
      order_ptr; // and now the current order becomes the tail,;

    priceLevel.numberOfOrders += 1;
    priceLevel.totalVolume += static_cast<uint64_t>(ord.quantity) * ord.price;

    // just adding the order coordinates to the map;
    orderArray[ord.id] = {order_ptr, &priceLevel};
  }
}

void LOB::cancelOrder(uint64_t id) {
  if (id >= orderArray.size()) return;
  auto [ordptr, level] = orderArray[id];
  if (!ordptr) return;

  ordptr->prev->next = ordptr->next;
  if (ordptr->next) ordptr->next->prev = ordptr->prev;
  else level->tail = ordptr->prev;

  level->numberOfOrders--;
  level->totalVolume -= static_cast<uint64_t>(ordptr->price) * ordptr->quantity;

  orderArray[id] = {nullptr, nullptr};
  arena.deallocate(ordptr);
}

// This forces the compiler to generate the code in this .cpp file

