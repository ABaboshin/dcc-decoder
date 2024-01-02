#include "DccDevice.h"
#include "DccDecoder.h"

void DccDevice::SetDecoder (const DccDecoder& decoder)
{
    this->decoder = std::make_shared<DccDecoder>(decoder);
}

DccDevice::DccDevice(std::uint16_t address)
{
    this->address = address;
}

bool DccDevice::AddressMatched(const std::vector<std::uint8_t>& data, std::uint8_t addressLength)
{
    addressLength = 1;
    return (data[0] == 0 || data[0] == address);
}