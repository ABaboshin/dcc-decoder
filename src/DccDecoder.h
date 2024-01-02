#pragma once
#include <cstdint>
#include <vector>
#include "DccDecoderState.h"

class DccDecoder {
private:
    DccDecoderState state;
    std::uint64_t lastRise;

    virtual std::uint64_t now() = 0;
    void onBitReceived(uint8_t bit);

    void process(std::vector<std::uint8_t> data);

public:
    DccDecoder ();

    void OnRised();

    void OnFalled();

    virtual void Start() = 0;
};