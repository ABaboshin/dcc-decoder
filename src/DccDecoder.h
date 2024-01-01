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
    void onBitReceived(uint8_t bit) {
        switch (state.Status)
        {
        case DCCStatus::WAIT_PREAMBLE:
            if (bit == 1) state.PreambleOneBitCount++;
            else {
                if (state.PreambleOneBitCount >= 10 && state.PreambleOneBitCount <= 12)
                {
                    state.Status = DCCStatus::WAIT_FOR_DATA;
                    state.Data.push_back(0);
                    state.DataCount = 0;
                }
                else
                {
                    state.Reset();
                }
            }
            break;
        case DCCStatus::WAIT_FOR_DATA:
            state.Data[state.Data.size() - 1] <<= 1;
            state.Data[state.Data.size() - 1] |= 1;
            state.DataCount++;
            if (state.DataCount > 8)
            {
                if (!bit)
                {
                    state.DataCount = 0;
                    state.Data.push_back(0);
                    if (state.Data.size() > 6) state.Reset();
                } else {
                    process(state.Data);
                    state.Reset();
                }
            }
        break;
        default:
            break;
        }
    }

    void process(std::vector<std::uint8_t> data) 
    {

    }
    
public:
    DccDecoder () 
    {
        state.Reset();
    }

    void OnRised()
    {
        lastRise = now();
    }

    // https://www.nmra.org/sites/default/files/standards/sandrp/pdf/s-9.1_electrical_standards_for_digital_command_control_2021.pdf
    void OnFalled() {
        if (lastRise > 0)
        {
            const auto pulseLength = now() - lastRise;
            if (pulseLength >= 52 && pulseLength <= 64)
            {
                onBitReceived(1);
            }
            if (pulseLength >= 90 && pulseLength <= 10000)
            {
                onBitReceived(0);
            }
        }
    }
    virtual void Start() = 0;
};