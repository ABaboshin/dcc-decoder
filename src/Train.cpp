#include "Train.h"

Train::Train(std::uint16_t address) : DccDevice(address)
{
    this->address = address;
}

void Train::Process(const std::span<std::uint8_t>& data)
{
}