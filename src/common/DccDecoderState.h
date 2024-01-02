#pragma once
#include <vector>
#include "DccStatus.h"

class DccDecoderState
{
public:
    DccStatus Status;
    int PreambleOneBitCount;
    int DataCount;
    std::vector<std::uint8_t> Data;

    void Reset();
};