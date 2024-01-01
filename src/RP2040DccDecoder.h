#pragma once

#include "DccDecoder.h"
#include "pico/stdlib.h"

class RP2040DccDecoder : DccDecoder<uint64_t>
{
private:
    uint64_t now ();
public:
    void Start();
};