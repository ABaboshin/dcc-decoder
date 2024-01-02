#pragma once

#include "DccDecoder.h"
#include "pico/stdlib.h"

class RP2040DccDecoder : DccDecoder
{
private:
    bool default1;
    int pin;
    uint64_t now ();
public:
    void Start();
    RP2040DccDecoder(int pin, bool default1);
    friend void onfall(uint gpio, uint32_t events);
    friend void onrise(uint gpio, uint32_t events);
};