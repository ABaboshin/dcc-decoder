#pragma once
#include <cstdint>
#include "DccDevice.h"

class Train : DccDevice
{
public:
    Train(std::uint16_t address);
    void Process(const std::span<std::uint8_t>& data) override;
};