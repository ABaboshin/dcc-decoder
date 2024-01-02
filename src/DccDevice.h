#pragma once
#include <vector>

class DccDevice
{
    virtual void process(std::vector<std::uint8_t> data) = 0;
};