#pragma once
#include "Arena2.hpp"
#include "Order2.hpp"
#include <boost/circular_buffer.hpp>
#include <boost/circular_buffer/base.hpp>
#include <list>
#include <map>
#include <sys/types.h>
#include <unordered_map>
#include<array>
#include <bit>
#include <vector>
struct PriceList {
  Order sentinel;
  Order *head = nullptr; // contains the pointer to the highest priority order
  Order *tail = nullptr; // contains the pointer to the latest Order
  PriceList() {
	sentinel.quantity = 0;
	sentinel.next = nullptr;
	sentinel.prev = nullptr;
	head = &sentinel;
	tail = &sentinel;
  }
  u_int32_t numberOfOrders = 0;
  u_int64_t totalVolume = 0;

  void nullify(){
	  head = &sentinel;
	  tail = &sentinel;
	  numberOfOrders = 0;
	  totalVolume=0;}
};
class LOB {
	
public:
	LOB(){matchedList.reserve(10000);}
  // Core API
enum class Side : uint8_t{
	Buy,
	Sell
};
void processOrder(Order& ord);
template<Side S>
int indexBestPriceFinder();
  template<Side S>
  void matching(Order &ord);
  void addOrder(Order &ord);
  void cancelOrder(uint64_t id);
  uint64_t editOrder(uint64_t id, uint32_t newPrice, uint32_t newQuantity);

  void reset();

  // Utilities
  uint64_t generateID();
  void seeBidRank() const;
  void seeAskRank() const;

private:
  // price level with orders queued
  std::array<u_int64_t,5> bitIndexAsk;
  std::array<u_int64_t,5> bitIndexBid;
  std::array<PriceList,300> priceMapAsk;
  std::array<PriceList,300> priceMapBid;
// Maps ID to the specific position in the price level list to retrieve Orders
  // faster.
  struct orderEntry {
    Order *ordptr;
    PriceList *levelptr;
  };
  std::unordered_map<uint64_t, orderEntry> orderMap;
  std::array<orderEntry,20002> orderArray;

  std::vector<matchResult> matchedList;
  static constexpr auto daily_size = 1024 * 1024 * 10;
  Arena arena{daily_size};
  u_int64_t NextID = 0;
};

using Side = LOB::Side;

template<Side S>
int LOB::indexBestPriceFinder(){
	int index = -1;
	if constexpr (S == Side::Buy){
		auto& bitMapToLookInto = bitIndexBid;
		for(int i = 4; i >= 0 ; i--){
			auto leadingz = std::countl_zero(bitMapToLookInto[i]) ;
			if( leadingz != 64 ){
				index = (i)*(64) + (63-leadingz);
				break;
			}
			
		}
	}

	else{
		auto& bitMapToLookInto = bitIndexAsk;
		for(int i = 0 ; i<=4;i++){
			auto leadingz = std::countr_zero(bitMapToLookInto[i]) ;
			if( leadingz != 64 ){
				index = (i)*64+(leadingz);
				break;
			}

	}}
	return index;
};



template<LOB::Side S>
void LOB::matching(Order &ord) {
constexpr Side oppositeSide = S == Side::Buy ? Side::Sell :Side::Buy;
auto& oppositeMap = (S == Side::Buy) ? priceMapAsk : priceMapBid;
auto& oppositeBitIndexMap = (S == Side::Buy) ? bitIndexAsk : bitIndexBid;
auto bestIndex = indexBestPriceFinder<oppositeSide>();
auto isIndexValid = [&](int currentIndex){
    if (currentIndex == -1 || currentIndex >= 300) return false;
    if constexpr (S == Side::Buy){
        return currentIndex <= ord.price;
    }
    else {
        return currentIndex >= ord.price;
    }
};

  while (ord.quantity > 0 && isIndexValid(bestIndex)) {
    auto &lstPriceLevel = oppositeMap[bestIndex];
    if (lstPriceLevel.head->next == nullptr) {
        oppositeBitIndexMap[bestIndex >> 6] &= ~(1ULL << (bestIndex & 63));
        bestIndex = indexBestPriceFinder<oppositeSide>();
        continue; 
    };
    // we drain the orders at the found price level :
    while (ord.quantity > 0 && lstPriceLevel.head->next != nullptr) {
      auto &matchedOrder = *lstPriceLevel.head->next;
      uint32_t qmatched = std::min(ord.quantity, matchedOrder.quantity);
      ord.quantity -= qmatched;
      matchedOrder.quantity -= qmatched;

      // Metadata draining logic
      lstPriceLevel.totalVolume -=
          static_cast<uint64_t>(qmatched) * bestIndex;

      matchedList.push_back(
          {ord.id, matchedOrder.id, static_cast<uint32_t>(bestIndex), qmatched});
      // removal and update logic
      if (matchedOrder.quantity == 0) {
        lstPriceLevel.numberOfOrders -= 1; // Metadata draining logic
	//freelist logic
	matchedOrder.enabled = false;
	

        orderArray[matchedOrder.id] = orderEntry(nullptr,nullptr);
        auto &newHead = matchedOrder.next;
        if (newHead != nullptr) {
          newHead->prev = lstPriceLevel.head;
	  lstPriceLevel.head->next = newHead;
        }
	else { // there's no Order left at this level so we nullify the sentinel next;
          (lstPriceLevel).tail = lstPriceLevel.head;
	  lstPriceLevel.head->next = nullptr;
	  oppositeBitIndexMap[matchedOrder.price/64] &= ~(1ULL << (matchedOrder.price%64));
        }
	arena.deallocate(&matchedOrder);
      }
    }
     bestIndex= indexBestPriceFinder<oppositeSide>();


      }
}

