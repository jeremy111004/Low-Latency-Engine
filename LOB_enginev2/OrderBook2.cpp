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
    Order *order_ptr = new (arena.allocate<Order>()) Order(ord);
    
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
  auto exist = orderMap.find(id);
  if (exist == orderMap.end())
    return;
  auto ordEntry = exist->second;
  auto ordptr = ordEntry.ordptr;
  auto listAtPriceLevel = ordEntry.levelptr;

  // Neighbor linking
  if (ordptr->prev) {
    ordptr->prev->next = ordptr->next;
  } else { // if there's no prev it was a head
    listAtPriceLevel->head =
        ordptr->next; // so the new head is either nullptr or ordptr->next;
  }
  if (ordptr->next) {
    ordptr->next->prev = ordptr->prev;
  } else { // if there's no next then it was a tail
    listAtPriceLevel->tail = ordptr->prev;
  }

  // we decrement the metadata of the price level
  listAtPriceLevel->numberOfOrders--;
  // Use 64 bit promotion to prevent overflow before subtraction
  listAtPriceLevel->totalVolume -=
      static_cast<uint64_t>(ordptr->price) * ordptr->quantity;
  arena.deallocate(ordptr);
  

  orderMap.erase(id);
}



// This forces the compiler to generate the code in this .cpp file

