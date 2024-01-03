#include "Train.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

Train::Train(std::uint16_t address, std::uint8_t fwdPin, std::uint8_t revPin) : DccDevice(address)
{
    this->fwdPin = fwdPin;
    this->revPin = revPin;

    gpio_init(fwdPin);
    gpio_init(revPin);

    initMotorPwm(fwdPin);
    initMotorPwm(revPin);

    // pwm_set_gpio_level(fwdPin, 0);
    // pwm_set_gpio_level(revPin, 1);

    // gpio_init(fwdPin);
    // gpio_init(revPin);
    // gpio_set_dir(fwdPin, GPIO_OUT);
    // gpio_set_dir(revPin, GPIO_OUT);
    // gpio_put(fwdPin, 1);
    // gpio_put(revPin, 0);
}

void Train::initMotorPwm(std::uint8_t pin)
{
    gpio_set_function(pin, GPIO_FUNC_PWM);
    const auto wrap = 4999;
    const auto slice = pwm_gpio_to_slice_num(pin);
    pwm_set_clkdiv_int_frac(slice, 1, 0);
    pwm_set_wrap(slice, wrap);
    pwm_set_enabled(slice, true);
    pwm_set_gpio_level(pin, 0);
}

void Train::Process(const std::span<std::uint8_t>& data)
{
    const auto isDirectionCommand = (data[0] & 0b00011111) > 0;
    if (isDirectionCommand)
    {
        const auto direction = data[0] & 0b10000000;
        const auto speed = data[0] & 0b01111111;

        if (speed < 2)
        {
            pwm_set_gpio_level(fwdPin, 0);
            pwm_set_gpio_level(revPin, 0);
        }

        if (direction)
        {
            pwm_set_gpio_level(revPin, 0);
            pwm_set_gpio_level(fwdPin, 4999 / speed * 128);
        } else {
            pwm_set_gpio_level(fwdPin, 0);
            pwm_set_gpio_level(revPin, 4999 / speed * 128);
        }
    }
}