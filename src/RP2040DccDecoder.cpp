#include "RP2040DccDecoder.h"

uint64_t RP2040DccDecoder::now()
{
    return time_us_64();
}