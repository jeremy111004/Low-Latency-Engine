#pragma once
#include <cstdint>
#include <array>


struct SoA {	
	// daily size parameters
	static constexpr uint16_t size = 300;
	static constexpr uint8_t maxQueueDepth = 64;
	static constexpr uint16_t dailyOrdersQuantity = 20000;

	// price level enabled maps
	std::array<uint64_t,5> bitIndexAsk{};
	std::array<uint64_t,5> bitIndexBid{};

	// actual queued orders by price level 
	uint32_t idQueuedAsk[size][maxQueueDepth]{};
	uint32_t idQueuedBid[size][maxQueueDepth]{};
	
	//quantity per order
	uint16_t quantityOrdersAsk[size][maxQueueDepth]{};
	uint16_t quantityOrdersBid[size][maxQueueDepth]{};

	// current heads and tails
	uint8_t askHead[size]{};
	uint8_t bidHead[size]{};
	uint8_t askTail[size]{};
	uint8_t bidTail[size]{};


	char typeByID[dailyOrdersQuantity]{};
	uint16_t priceByID[dailyOrdersQuantity]{};
	uint8_t physicalLocByID[dailyOrdersQuantity];

};

struct matchResult {	
  uint64_t id1;
  uint64_t id2;
  uint32_t quantity;
  uint32_t price;
};

enum class Side : uint8_t{
	Buy,
	Sell
};
