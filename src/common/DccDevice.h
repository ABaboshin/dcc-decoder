#pragma once
#include <vector>
#include <span>
#include <memory>

class DccDecoder;

class DccDevice
{
protected:
    std::shared_ptr<DccDecoder> decoder;
    std::uint16_t address;

public:
    DccDevice(std::uint16_t address);
    void SetDecoder (const DccDecoder& decoder);
    bool AddressMatched(const std::vector<std::uint8_t>& data, std::uint8_t addressLength);
    virtual void Process(const std::span<std::uint8_t>& data) {};
};