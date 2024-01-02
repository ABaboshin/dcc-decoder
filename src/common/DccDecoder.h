#pragma once
#include <cstdint>
#include <vector>
#include "DccDecoderState.h"
#include "DccDevice.h"

class DccDecoder {
private:
    DccDecoderState state;
    std::uint64_t lastRise;

    virtual std::uint64_t now();

    void onBitReceived(uint8_t bit);

    void process();

    std::vector<DccDevice> devices;

public:
    DccDecoder ();

    void OnRised();

    void OnFalled();

    virtual void Start() {};

    void Register(const DccDevice& device);
};