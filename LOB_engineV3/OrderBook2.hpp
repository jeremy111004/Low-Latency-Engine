#pragma once

#include "Order2.hpp"
#include <boost/circular_buffer.hpp>
#include <boost/circular_buffer/base.hpp>

#include <sys/types.h>

#include<array>
#include <bit>
#include <vector>

class LOB {
	
public:
	LOB(){matchedList.reserve(10000);}
  // Core API
enum class Side : uint8_t{
	Buy,
	Sell
};
void processOrder(uint16_t quantity, uint32_t ID, uint16_t price, Side type);
template<Side S>
int indexBestPriceFinder();
  template<Side S>
  uint16_t matching(uint16_t quantity, uint32_t ID, uint16_t price);

  template<Side S>
  void addOrder(uint16_t quantity, uint32_t ID, uint16_t price);
  void cancelOrder(uint32_t ID);
  uint64_t editOrder(uint64_t id, uint32_t newPrice, uint32_t newQuantity);

  void reset();

  // Utilities
  uint64_t generateID();
 

private:
   SoA data; 
   std::vector<matchResult> matchedList;
   uint16_t NextID{0} ;
  };

using Side = LOB::Side;

template<Side S>
int LOB::indexBestPriceFinder(){
	int index = -1;
	if constexpr (S == Side::Buy){
		auto& bitMapToLookInto = data.bitIndexBid;
		for(int i = 4; i >= 0 ; i--){
			auto leadingz = std::countl_zero(bitMapToLookInto[i]) ;
			if( leadingz != 64 ){
				index = (i)*(64) + (63-leadingz);
				break;
			}
			
		}
	}

	else{
		auto& bitMapToLookInto = data.bitIndexAsk;
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
uint16_t LOB::matching(uint16_t quantity, uint32_t ID, uint16_t price) {

constexpr Side oppositeSide = S == Side::Buy ? Side::Sell :Side::Buy;
auto bestIndex = indexBestPriceFinder<oppositeSide>();

auto& oppositeMapID = (S == Side::Buy) ? data.idQueuedAsk : data.idQueuedBid;
auto& oppositeMapQ = (S == Side::Buy) ? data.quantityOrdersAsk : data.quantityOrdersBid;
auto& oppositeBitIndexMap = (S == Side::Buy) ? data.bitIndexAsk : data.bitIndexBid;
auto& oppositeMapHead = S == Side::Buy ? data.askHead : data.bidHead;
auto& oppositeMapTail = S == Side::Buy ? data.askTail : data.bidTail;


auto isIndexValid = [&](int currentIndex){
    if (currentIndex == -1 || currentIndex >= 300) return false;
    if constexpr (S == Side::Buy){
        return currentIndex <= price;
    }
    else {
        return currentIndex >= price;
    }
};

  while (quantity > 0 && isIndexValid(bestIndex)) {
    
    auto &lstPriceLevelID = oppositeMapID[bestIndex];
    auto &lstPriceLevelQ = oppositeMapQ[bestIndex];
    auto& highestPrioOrderIndex = oppositeMapHead[bestIndex];
    auto& lowestPrioOrderIndex = oppositeMapTail[bestIndex];

    //SAFEGUARD cleaning the bitIndexMap
    if (highestPrioOrderIndex == lowestPrioOrderIndex) {
        oppositeBitIndexMap[bestIndex >> 6] &= ~(1ULL << (bestIndex & 63));
        bestIndex = indexBestPriceFinder<oppositeSide>();
        continue; 
    };

    // we drain the orders at the found price level :
    while (quantity > 0 && highestPrioOrderIndex != lowestPrioOrderIndex ) {
      auto &matchedOrderID = lstPriceLevelID[highestPrioOrderIndex & 63] ;
      auto &matchedOrderQ = lstPriceLevelQ[highestPrioOrderIndex & 63];
      if(matchedOrderID == 0)[[unlikely]]{highestPrioOrderIndex++; continue;}
      uint32_t qmatched = std::min(quantity, matchedOrderQ);
      quantity -= qmatched;
      matchedOrderQ -= qmatched;

           matchedList.push_back(
          {ID, matchedOrderID, static_cast<uint32_t>(bestIndex), qmatched});
      // removal and update logic
      if (matchedOrderQ == 0) {
       
	data.priceByID[matchedOrderID]       = 300;
	data.physicalLocByID[matchedOrderID] = 64;  
	data.typeByID[matchedOrderID]        = 'I';

	 matchedOrderID = 0;

	
	highestPrioOrderIndex ++;}
    

      }
	bestIndex= indexBestPriceFinder<oppositeSide>();

    }

   return quantity; 
}
