#include "DccDecoder.h"

DccDecoder::DccDecoder ()
{
    state.Reset();
}

std::uint64_t DccDecoder::now()
{
    return 0;
}

// https://www.nmra.org/sites/default/files/standards/sandrp/pdf/s-9.1_electrical_standards_for_digital_command_control_2021.pdf
void DccDecoder::OnFalled()
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

void DccDecoder::OnRised()
{
    lastRise = now();
}

void DccDecoder::onBitReceived(uint8_t bit)
{
    switch (state.Status)
    {
    case DccStatus::WAIT_PREAMBLE:
        if (bit == 1) state.PreambleOneBitCount++;
        else {
            if (state.PreambleOneBitCount >= 10 && state.PreambleOneBitCount <= 12)
            {
                state.Status = DccStatus::WAIT_FOR_DATA;
                state.Data.push_back(0);
                state.DataCount = 0;
            }
            else
            {
                state.Reset();
            }
        }
        break;
    case DccStatus::WAIT_FOR_DATA:
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
                process();
                state.Reset();
            }
        }
    break;
    default:
        break;
    }
}

void DccDecoder::Register(const DccDevice& device)
{
    devices.push_back(device);
}

void DccDecoder::process()
{
    for (auto &it: devices)
    {
        std::uint8_t addressLength;
        if (it.AddressMatched(state.Data, addressLength))
        {
            it.Process(std::span(state.Data.begin() + addressLength, state.Data.end()));
        }
    }
}