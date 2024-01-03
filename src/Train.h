#pragma once
#include <cstdint>
#include "DccDevice.h"

class Train : DccDevice
{
    std::uint8_t fwdPin;
    std::uint8_t revPin;

    void initMotorPwm(std::uint8_t pin);
public:
    Train(std::uint16_t address, std::uint8_t fwdPin, std::uint8_t revPin);
    void Process(const std::span<std::uint8_t>& data) override;
};