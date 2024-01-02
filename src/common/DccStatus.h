#pragma once
#include <cstdint>

enum DccStatus : std::uint8_t
{
    WAIT_PREAMBLE,
    WAIT_FOR_ADDRESS,
    WAIT_FOR_DATA
};