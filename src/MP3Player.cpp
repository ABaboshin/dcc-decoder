#include <iostream>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "pico.h"
#include "hardware/structs/pwm.h"
#include "hardware/regs/dreq.h"
#include "ff.h"
extern "C"
{
    #include "interface/music_file.h"
}

#include "MP3Player.h"

MP3Player::MP3Player(std::uint8_t left, std::uint8_t right)
{
    std::cout << "MP3Player" << std::endl;
    this->left.Pin = left;
    this->right.Pin = right;

    InitChannel(this->left);
    InitChannel(this->right);

    // int mask = 0;
    // mask |= 0x01 << this->left.DmaChannel;
    // mask |= 0x01 << this->right.DmaChannel;
    // dma_set_irq1_channel_mask_enabled(mask, true);
    // irq_set_enabled(DMA_IRQ_1, true);
}

void MP3Player::InitChannel(PwmData& data)
{
    std::cout << "MP3Player::InitChannel" << std::endl;
    auto config = pwm_get_default_config();
    gpio_set_function(data.Pin, GPIO_FUNC_PWM);
    data.PinSlice = pwm_gpio_to_slice_num(data.Pin);
    pwm_init(data.PinSlice, &config, false);

    // auto dmaConfig = dma_channel_get_default_config(data.DmaChannel);
    // channel_config_set_read_increment(&dmaConfig, true); 
    // channel_config_set_write_increment(&dmaConfig, false); 
    // channel_config_set_dreq(&dmaConfig, DREQ_PWM_WRAP0 + data.PinSlice); 
    // channel_config_set_transfer_data_size(&dmaConfig, DMA_SIZE_32); 
    // channel_config_set_chain_to(&dmaConfig, data.DmaChannel);

    // // Set up config
    // dma_channel_configure(data.DmaChannel, 
    //                       &dmaConfig, 
    //                       &pwm_hw->slice[data.PinSlice].cc, 
    //                       data.DmaBuffer,
    //                       2200,
    //                       false);
}

#define CACHE_BUFFER 8000
unsigned char cache_buffer[CACHE_BUFFER];

// extern char output_mp3_start[] asm( "_binary_output_mp3_start" );
// extern char output_mp3_end[]   asm( "_binary_output_mp3_end" );
// extern size_t output_mp3_size  asm( "_binary_output_mp3_size" );

#include "resource.cpp"

void MP3Player::Play()
{
    std::cout << "MP3Player::Play" << std::endl;
    FIL fil;
    music_file mf;
    mf.fil = fil;
    std::cout << "size " << sizeof(outputmp3) << std::endl;
    std::cout << musicFileCreate(&mf, "test.mp3", cache_buffer, CACHE_BUFFER) << std::endl;
}
