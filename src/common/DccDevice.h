#pragma once
#include <vector>
#include <memory>

class DccDecoder;

class DccDevice
{
    virtual void process(std::vector<std::uint8_t> data) {};
    std::shared_ptr<DccDecoder> decoder;

public:
    void SetDecoder (const DccDecoder& decoder);
};