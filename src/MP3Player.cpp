#include <iostream>
#include <memory>
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

std::shared_ptr<MP3Player> player;
music_file mf;

void Stop()
{
    pwm_set_enabled(player->left.PinSlice, false);
    // pwm_set_enabled(player->right.PinSlice, false);
    dma_channel_abort(player->left.PinSlice);
    // dma_channel_abort(player->right.PinSlice);
}

void DmaIrqHandler()
{
    std::cout << "DmaIrqHandler" << std::endl;
    
    if (dma_channel_get_irq1_status(player->left.DmaChannel))
    {
        dma_channel_acknowledge_irq1(player->left.DmaChannel);
        dma_channel_set_read_addr(player->left.DmaChannel, player->left.DmaBuffer, false);

        std::uint32_t written;
        auto readRes = musicFileRead(&mf, player->left.DmaBuffer, DMA_BUFFER_SIZE, &written);
        std::cout << "readRes " << readRes << " written " << written << std::endl;

        if (written > 0)
        {
            dma_channel_set_read_addr(player->left.DmaChannel, player->left.DmaBuffer, true);
        } else {
            Stop();
        }
    }
}

MP3Player::MP3Player(std::uint8_t left, std::uint8_t right)
{
    std::cout << "MP3Player" << std::endl;
    this->left.Pin = left;
    // this->right.Pin = right;

    InitChannel(this->left);
    // InitChannel(this->right);

    irq_set_exclusive_handler(DMA_IRQ_1, DmaIrqHandler);

    int mask = 0;
    mask |= 0x01 << this->left.DmaChannel;
    // mask |= 0x01 << this->right.DmaChannel;

    dma_set_irq1_channel_mask_enabled(mask, true);
    irq_set_enabled(DMA_IRQ_1, true);

    player = std::make_shared<MP3Player>(*this);
}

void MP3Player::InitChannel(PwmData& data)
{
    std::cout << "MP3Player::InitChannel" << std::endl;

    auto config = pwm_get_default_config();
    gpio_set_function(data.Pin, GPIO_FUNC_PWM);
    data.PinSlice = pwm_gpio_to_slice_num(data.Pin);

    pwm_config_set_clkdiv(&config, 1.0f); 
    pwm_config_set_wrap(&config, 4082); 

    pwm_init(data.PinSlice, &config, false);

    std::cout << "dma_claim_unused_channel" << std::endl;
    data.DmaChannel = dma_claim_unused_channel(true);

    std::cout << "dma_channel_get_default_config" << std::endl;

    auto dmaConfig = dma_channel_get_default_config(data.DmaChannel);
    channel_config_set_read_increment(&dmaConfig, true); 
    channel_config_set_write_increment(&dmaConfig, false); 
    channel_config_set_dreq(&dmaConfig, DREQ_PWM_WRAP0 + data.PinSlice); 
    channel_config_set_transfer_data_size(&dmaConfig, DMA_SIZE_32); 
    channel_config_set_chain_to(&dmaConfig, data.DmaChannel);

    std::cout << "dma_channel_configure" << std::endl;

    // Set up config
    dma_channel_configure(data.DmaChannel, 
                          &dmaConfig, 
                          &pwm_hw->slice[data.PinSlice].cc, 
                          data.DmaBuffer,
                          22000,
                          false);
}

#define CACHE_BUFFER 8000
unsigned char cache_buffer[CACHE_BUFFER];

// extern char output_mp3_start[] asm( "_binary_output_mp3_start" );
// extern char output_mp3_end[]   asm( "_binary_output_mp3_end" );
// extern size_t output_mp3_size  asm( "_binary_output_mp3_size" );

#include "resource.cpp"

void PopulateDmaBuffer()
{
    std::uint32_t written;
    auto readRes = musicFileRead(&mf, player->left.DmaBuffer, DMA_BUFFER_SIZE, &written);
    std::cout << "readRes " << readRes << " written " << written << std::endl;
}

void MP3Player::Play()
{
    std::cout << "MP3Player::Play" << std::endl;
    FIL fil;
    fil.data = outputmp3;
    fil.size = sizeof(outputmp3);
    fil.currentPosition = 0;
    
    mf.fil = fil;
    std::cout << "size " << sizeof(outputmp3) << std::endl;
    std::cout << musicFileCreate(&mf, "test.mp3", cache_buffer, CACHE_BUFFER) << std::endl;

    auto sampleRate = musicFileGetSampleRate(&mf);
    auto isStereo = musicFileIsStereo(&mf);

    std::cout << "sampleRate " << sampleRate << std::endl;

    // auto shift = 0;
    // auto wrap = 4082;
    // auto fraction = 1.0f;

    // auto ram_buffer_index = 0;
    // auto dma_buffer_index = 0;

    /*
    doubleBufferInitialise(&double_buffers, &populateCallback, &current_RAM_Buffer, &current_RAM_length);

    // Populate the first DMA buffer
    PopulateDmaBuffer();

    */

   PopulateDmaBuffer();

   auto pwmMask = 0;
   pwmMask |= 0x01 << left.PinSlice;
//    pwmMask |= 0x01 << right.PinSlice;

    auto dnaMask = 0x01 << this->left.DmaChannel;
    dma_start_channel_mask(dnaMask);
    pwm_set_mask_enabled(pwmMask);

    std::cout << "started " << std::endl;
}
