#pragma once
// #include <cstdint>

// #define DMA_BUFFER_SIZE 22000

class PwmData
{
public:
    std::uint8_t PinSlice;
    std::uint8_t Pin;
    // std::int32_t DmaChannel;
    // std::int16_t DmaBuffer[DMA_BUFFER_SIZE];
};

// class MP3Player
// {
//     PwmData left;
//     PwmData right;

//     void InitChannel(PwmData& data);
// public:
//     MP3Player(std::uint8_t left, std::uint8_t right);
//     void Play();

//     friend void DmaIrqHandler();
//     friend void PopulateDmaBuffer();
//     friend void Stop();
// };