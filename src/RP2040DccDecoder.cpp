#include "RP2040DccDecoder.h"
#include "pico/stdlib.h"
#include <memory>

std::shared_ptr<RP2040DccDecoder> decoder;

uint64_t RP2040DccDecoder::now()
{
    return time_us_64();
}

RP2040DccDecoder::RP2040DccDecoder(int pin, bool default1)
{
    this->default1 = default1;
    this->pin = pin;
    decoder = std::make_shared<RP2040DccDecoder>(*this);

    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

void onfall(uint gpio, uint32_t events)
{
    if (decoder->default1) {
        decoder->OnRised();
    } else {
        decoder->OnFalled();
    }
}

void onrise(uint gpio, uint32_t events)
{
    if (decoder->default1) {
        decoder->OnFalled();
    } else {
        decoder->OnRised();
    }
}

void RP2040DccDecoder::Start()
{
    gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_FALL, false, &onfall);
    gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE, false, &onrise);
}
