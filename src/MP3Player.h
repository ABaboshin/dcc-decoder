#pragma once
#include <cstdint>

class PwmData
{
public:
    std::uint8_t PinSlice;
    std::uint8_t Pin;
    std::int32_t DmaChannel;
    std::uint32_t DmaBuffer[2200];
};

class MP3Player
{
    PwmData left;
    PwmData right;

    void InitChannel(PwmData& data);
public:
    MP3Player(std::uint8_t left, std::uint8_t right);
    void Play();
};