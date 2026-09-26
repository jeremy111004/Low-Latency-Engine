#pragma once
#include <vector>
#include <cstdint>
#include "Order2.hpp"

struct InboundJob {
    uint32_t orderId;
    uint16_t price;
    uint16_t quantity;
    Side     side;
};

std::vector<InboundJob> fileReaderParser(const char* filename) noexcept;
