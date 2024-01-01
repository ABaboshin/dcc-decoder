#include "DccDecoder.h"

void DccDecoderState::Reset ()
{
    Status = DCCStatus::WAIT_PREAMBLE;
    PreambleOneBitCount = 0;
    DataCount = 0;
    Data.clear();
}

// template<typename TimeT>
// DccDecoder<TimeT>::DccDecoder ()
// {
//     state.Reset();
// }

// https://www.nmra.org/sites/default/files/standards/sandrp/pdf/s-9.1_electrical_standards_for_digital_command_control_2021.pdf
template<typename TimeT>
void DccDecoder<TimeT>::OnFalled()
{
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

template<typename TimeT>
void DccDecoder<TimeT>::OnRised()
{
    lastRise = now();
}

template<typename TimeT>
void DccDecoder<TimeT>::onBitReceived(uint8_t bit)
{
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