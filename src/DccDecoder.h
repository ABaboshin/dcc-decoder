#pragma once
#include <cstdint>
#include <vector>

enum DCCStatus : std::uint8_t
{
    WAIT_PREAMBLE,
    WAIT_FOR_ADDRESS,
    WAIT_FOR_DATA
};

class DccDecoderState
{
public:
    DCCStatus Status;
    int PreambleOneBitCount;
    int DataCount;
    std::vector<std::uint8_t> Data;

    void Reset();
};

template<typename TimeT>
class DccDecoder {
private:
    DccDecoderState state;
    TimeT lastRise;

    virtual TimeT now() = 0;
    void onBitReceived(uint8_t bit);
    void process(std::vector<std::uint8_t> data);
public:
    DccDecoder () 
    {
        state.Reset();
    }

    void OnRised();
    void OnFalled();
    virtual void Start() = 0;
};