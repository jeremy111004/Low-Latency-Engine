#include "OrderBook2.hpp"
#include "Order2.hpp"

#include <cstdint>

#include <map>
#include <unordered_map>


uint64_t LOB::generateID() noexcept {
	return NextID++; 
}

void LOB::reset() noexcept {
   matchedList.clear();

   uint16_t numberElements = 64;
   uint16_t numberPriceLevel = 300;
   for(uint16_t i = 0; i < numberPriceLevel; i ++){
	   data.askHead[i] = 0;
	   data.askTail[i] = 0;
	   data.bidHead[i] = 0;
	   data.bidTail[i] = 0;
	   for(uint16_t j = 0; j < numberElements; j++){
		data.idQueuedAsk[i][j] = 0;
		data.idQueuedBid[i][j] = 0;
		data.quantityOrdersAsk[i][j] = 0;
		data.quantityOrdersBid[i][j] = 0;


	   }
   }
   std::fill(std::begin(data.bitIndexAsk),std::end(data.bitIndexAsk),0);
   std::fill(std::begin(data.bitIndexBid),std::end(data.bitIndexBid),0);
 
  NextID = 0;
 
}
 void LOB::processOrder(uint16_t quantity, uint32_t ID, uint16_t price, Side type) noexcept{

    uint16_t Qremaining{};

    if(price >= 300 || price < 1)[[unlikely]]{return;}

    if(type == Side::Sell){

        Qremaining = LOB::matching<Side::Sell>(quantity, ID, price)    ;    if(Qremaining > 0){addOrder<Side::Sell>(Qremaining,ID,price);}

    

    }

    else{

        Qremaining = LOB::matching<Side::Buy>(quantity, ID, price);

        if(Qremaining > 0){addOrder<Side::Buy>(Qremaining,ID,price) ;}


    }


};

template<Side S>

void LOB::addOrder(uint16_t quantity, uint32_t ID, uint16_t price) noexcept {

 // if the order hasn't been fully matched (or not matched at all) we have to

  // actually save it:

    constexpr Side side = S == Side::Buy ? Side::Buy : Side::Sell;     


    auto &priceLevelID = side == Side::Buy ? data.idQueuedBid[price] : data.idQueuedAsk[price];

    auto &priceLevelQ =side == Side::Buy ? data.quantityOrdersBid[price] : data.quantityOrdersAsk[price];

    auto &bitIndexMap = side == Side::Buy ? data.bitIndexBid : data.bitIndexAsk;

    auto& priceLvlTail = side == Side::Buy? data.bidTail[price] : data.askTail[price];

    auto& priceLvlHead = side == Side::Buy ? data.bidHead[price] : data.askHead[price];


    if(priceLvlTail - priceLvlHead == 64)[[unlikely]]{return;}


      bitIndexMap[price >> 6] |= 1ULL << (price & 63);


      

      priceLevelID[priceLvlTail & 63] = ID;

      priceLevelQ[priceLvlTail & 63] = quantity;

      data.typeByID[ID] = S == Side::Buy ? 'B' : 'A';

      data.priceByID[ID] = price;

      priceLvlTail ++;

  } 
void LOB::cancelOrder(uint32_t id) noexcept {
	//safeguard
	if(id > 20000)[[unlikely]] return;

	auto priceLevel = data.priceByID[id];
	auto typeByID = data.typeByID[id];
	auto phyLoc = data.physicalLocByID[id];

	// when Orders have been filled and not overwritten yet, their phyLoc are set at 64,
	// and price at 300 
	if(phyLoc > 63 || priceLevel>299)[[unlikely ]] return;
	auto& mapToCheck = typeByID == 'B' ? data.idQueuedBid[priceLevel] : data.idQueuedAsk[priceLevel];	

	typeByID == 'B' ? data.quantityOrdersBid[priceLevel][phyLoc] = 0 : data.quantityOrdersAsk[priceLevel][phyLoc] = 0;
	mapToCheck[phyLoc] = 0;

	data.priceByID[id]       = 300;
	data.physicalLocByID[id] = 64;  
	data.typeByID[id]        = 'I';



  
}



// This forces the compiler to generate the code in this .cpp file

