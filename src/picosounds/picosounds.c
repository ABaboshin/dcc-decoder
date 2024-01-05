#include <stdio.h>
#include <math.h>          // For fminf and fmaxf
#include "pico/stdlib.h"   // stdlib 
#include "hardware/irq.h"  // interrupts
#include "hardware/dma.h"  // dma 
#include "hardware/sync.h" // wait for interrupt 
#include "hardware/structs/ioqspi.h"
#include "pico/util/queue.h" 


#include "pwm_channel.h"
#include "double_buffer.h"
#include "ff.h"
#include "music_file.h"
#include "config.h"

#ifdef DEBUG_STATUS
  #define STATUS(a) printf a
#else
  #define STATUS(a) (void)0
#endif
 
#define AUDIO_PIN 10  // Configured for the Maker board 18 left, 19 right
#define STEREO        // When stereo not enabled, DMA same l and r data to both channels
#define VOLUME

#define IS_RGBW false
#define NUM_PIXELS 1

#ifdef STEREO
bool play_stereo = true;
#else
bool play_stereo = false;
#endif

#define SAMPLE_RATE 22000           // Used for coloured noise generation
#define DMA_BUFFER_LENGTH 2200      // 2200 samples @ 44kHz gives= 0.05 seconds = interrupt rate

#define RAM_BUFFER_LENGTH (4*DMA_BUFFER_LENGTH)

/*
 * Static variable definitions
 */
static uint wrap;                           // Largest value a sample can be + 1
static int mid_point;                       // wrap divided by 2
static float fraction = 1;                  // Divider used for PWM
static int repeat_shift = 1;                // Defined by the sample rate

static pwm_data pwm_channel[2];             // Represents the PWM channels
static int dma_channel[2];                  // The 2 DMA channels used for DMA ping pong

 // Have 2 buffers in RAM that are used to DMA the samples to the PWM engine
static uint32_t dma_buffer[2][DMA_BUFFER_LENGTH];
static int dma_buffer_index = 0;            // Index into active DMA buffer

// Have 2 or 4 8k buffers in RAM, copy data from Flash to these buffers - in future
// will be buffers where noise is created, or music delivered from SD Card

// RAM buffers, controlled through double_buffer class
static int16_t ram_buffer[2][RAM_BUFFER_LENGTH];
static bool sampled_stereo = false;         // True if ram_buffer contains stereo, false for mono

// Control data blocks for the RAM double buffers
static double_buffer double_buffers;
uint32_t populateCallback(int16_t* buffer, uint32_t len);   // Call back to generate next buffer of sound

// Working buffer for reading from file
#define CACHE_BUFFER 8000
unsigned char cache_buffer[CACHE_BUFFER];

// Pointer to the currently in use RAM buffer
static const int16_t* current_RAM_Buffer = 0;
static int ram_buffer_index = 0;            // Holds current position in ram_buffers for channels
static uint32_t current_RAM_length = 0;     // number of active samples in current RAM buffer

static float volume = 0.8;                  // Initial volume adjust, controlled by button

// Event queue, used to leave ISR context
static queue_t eventQueue;

// Supported events
typedef enum Event 
{
    empty = 0,
    increase_volume = empty + 1, 
    decrease_volume = increase_volume + 1,
    populate_dma = decrease_volume + 1,
    populate_double = populate_dma + 1,
    change_music = populate_double + 1,
    increase_intensity = change_music + 1, 
    decrease_intensity = increase_intensity + 1,
    change_led = decrease_intensity + 1,
    quit = change_led + 1, 
} Event; 

// To convert from 16 bit signed to unsigned
#define MID_VALUE 0x8000

static void changeState(sound_state new_state);
sound_state current_state = off; 

/* 
 * Function declarations
 */
static void populateDmaBuffer(void);
static void claimDmaChannels(int num_channels);
static void initDma(int buffer_index, int slice, int chain_index);
static void dmaInterruptHandler();

void startMusic(uint32_t sample_rate);
void stopMusic();
void exitMusic();

static bool loadFile(const char* filename);
static music_file mf;

/* 
 * Function definitions
 */

// Handles interrupts for the DMA chain
// Resets start address for DMA and requests buffer that is exhausted to be refilled
static void dmaInterruptHandler() 
{
    printf("dmaInterruptHandler\n");
    // Determine which DMA caused the interrupt
    for (int i = 0 ; i<2; ++i)
    {
        if (dma_channel_get_irq1_status(dma_channel[i]))
        {
            dma_channel_acknowledge_irq1(dma_channel[i]);
            dma_channel_set_read_addr(dma_channel[i], dma_buffer[i], false);

            // Populate buffer outside of IRQ
            Event e = populate_dma;
            queue_try_add(&eventQueue, &e);
        }
    }    
}

// Populate the DMA buffer, referenced by index
static void populateDmaBuffer(void)
{
    printf("populateDmaBuffer\n");
    uint32_t left;
    uint32_t right;

    // Calculate the wrap point for the ram_buffer_index
    uint32_t ram_buffer_wrap = (sampled_stereo) ? (current_RAM_length<<repeat_shift) : (current_RAM_length<<(repeat_shift+1));

    // Populate two bytes from each active buffer
    for (int i=0; i<DMA_BUFFER_LENGTH; ++i)
    {
        // build the 32 bit word from the two channels
        if (sampled_stereo)
        {
#ifdef VOLUME        
        // Write to buffer, adjusting for volume
            left = ((int32_t)(current_RAM_Buffer[(ram_buffer_index>>repeat_shift)<<1] * volume) + MID_VALUE) >> 4;
            right =((int32_t)(current_RAM_Buffer[((ram_buffer_index>>repeat_shift)<<1)+1] * volume) + MID_VALUE) >> 4;
#else
            left = (current_RAM_Buffer[(ram_buffer_index>>repeat_shift)<<1] + MID_VALUE) >> 4;
            right =(current_RAM_Buffer[((ram_buffer_index>>repeat_shift)<<1)+1] + MID_VALUE) >> 4;
#endif        
        }
        else
        {
#ifdef VOLUME        
        // Write to buffer, adjusting for volume
            left = ((int32_t)(current_RAM_Buffer[ram_buffer_index>>repeat_shift] * volume) + MID_VALUE) >> 4;
#else            
            left = (current_RAM_Buffer[ram_buffer_index>>repeat_shift] + MID_VALUE) >> 4;
#endif            
            right = left;
        }
        ram_buffer_index++;

        if (!play_stereo)
        {
            // Want mono, so average two channels
            left = (left + right) >> 1;
            right = left;
        }

        // Combine the two channels
        dma_buffer[dma_buffer_index][i] = (right << 16) + left;

        if ((ram_buffer_index<<1) == ram_buffer_wrap) 
        {
            // Need a new RAM buffer
            doubleBufferGetLast(&double_buffers, &current_RAM_Buffer, &current_RAM_length);

            // reset read position of RAM buffer to start
            ram_buffer_index = 0;

            // Signal to populate a new RAM buffer
            Event e = populate_double;
            queue_try_add(&eventQueue, &e);
        }
    }
    dma_buffer_index = 1 - dma_buffer_index;
}

// Obtain the DMA channels - need 2 
static void claimDmaChannels(int num_channels)
{
    for (int i=0; i<num_channels; ++i)
    {
        dma_channel[i] = dma_claim_unused_channel(true); 
    }
}

// Configure the DMA channels - including chaining
static void initDma(int buffer_index, int slice, int chain_index)
{
    dma_channel_config config = dma_channel_get_default_config(dma_channel[buffer_index]); 
    channel_config_set_read_increment(&config, true); 
    channel_config_set_write_increment(&config, false); 
    channel_config_set_dreq(&config, DREQ_PWM_WRAP0 + slice); 
    channel_config_set_transfer_data_size(&config, DMA_SIZE_32); 
    channel_config_set_chain_to(&config, dma_channel[chain_index]);

    // Set up config
    dma_channel_configure(dma_channel[buffer_index], 
                          &config, 
                          &pwm_hw->slice[slice].cc, 
                          dma_buffer[buffer_index],
                          DMA_BUFFER_LENGTH,
                          false);
}

int mainsound() 
{
    // Overclock to 180MHz so that system clock is a multiple of typical
    // audio sampling rates
    bool clock_set = set_sys_clock_khz(180000, true);
    
    // Adjust frequency before initialising, so serial port will work
    stdio_init_all();

    // Now stdio is available, check the frequency was set
    if (!clock_set)
    {
        printf("Cannot set clock rate\n");
        return -1;
    }   

    // Set up the PWMs with arbitrary values, will be updated when play starts
    pwmChannelInit(&pwm_channel[0], AUDIO_PIN);
    pwmChannelInit(&pwm_channel[1], AUDIO_PIN+1);

    // Get the DMA channels for the chain
    claimDmaChannels(2);

    // Initialise and Chain the two DMAs together
    initDma(0, pwmChannelGetSlice(&pwm_channel[0]), 1);
    initDma(1, pwmChannelGetSlice(&pwm_channel[0]), 0);

    // Set the DMA interrupt handler
    irq_set_exclusive_handler(DMA_IRQ_1, dmaInterruptHandler); 

    // Enable the interrupts for both of the chained dma channels
    int mask = 0;

    for (int i=0;i<2;++i)
    {
        mask |= 0x01 << dma_channel[i];
    }

    dma_set_irq1_channel_mask_enabled(mask, true);
    irq_set_enabled(DMA_IRQ_1, true);

    // Create the event queue
    Event event = empty;
    queue_init(&eventQueue, sizeof(event), 6);

    // Get the initial states
    sound_state new_state = change_music;
    
    // Use the initial states
    changeState(new_state);

    /*
     * Main loop 
     */
    while (true)
    {
        queue_remove_blocking(&eventQueue, &event);
        
        switch (event)
        {
            case change_music:
                changeState(current_state + 1);
            break;

            case increase_volume:
                volume = fminf(1.0, volume+0.1);
                // configSetVolume(&mount, volume);
            break;

            case decrease_volume:
                volume = fmaxf(0.0, volume-0.1);
                // configSetVolume(&mount, volume);
            break;

            case populate_dma:
                populateDmaBuffer();
            break;

            case populate_double:
                doubleBufferPopulateNext(&double_buffers);
            break;

            case quit:
                exitMusic();
            break;

            default:
                return -1;
            break;
        }
    }
    return 0;
}

static void changeState(sound_state new_state)
{
    // Handle wrap
    if (new_state == end)
    {
        new_state = start;
    }

    // Stop playing if we are, and close the file if it is open
    if (current_state != off)
    {
        stopMusic();

        musicFileClose(&mf);
    }

    loadFile("");



    // Handle the case where failure to open a file results in a wrap
    if (new_state == end)
    {
        new_state = start;
    }

    // State needs to be changed before buffers populated
    current_state = new_state;


    // Now in a position to start playing the sound
    uint32_t sample_rate;

    STATUS(("Sample rate is %u\n", mf.sample_rate));
    sample_rate = musicFileGetSampleRate(&mf);
    sampled_stereo = musicFileIsStereo(&mf);
    startMusic(sample_rate);
}

void startMusic(uint32_t sample_rate)
{ 
    Event skip = empty;

    // Reconfigure the PWM for the new wrap and clock
    getSampleValues(sample_rate, &repeat_shift, &wrap, &mid_point, &fraction);
    pwmChannelReconfigure(&pwm_channel[0], fraction, wrap);
    pwmChannelReconfigure(&pwm_channel[1], fraction, wrap);

    // reset read positions
    ram_buffer_index = 0;
    dma_buffer_index = 0;

    // Reinitialise the double buffers
    doubleBufferInitialise(&double_buffers, &populateCallback, &current_RAM_Buffer, &current_RAM_length);

    // Populate the first DMA buffer
    populateDmaBuffer();

    // Empty the message queue, to avoid processing populate messages
    while(queue_try_remove(&eventQueue, &skip));

    // Populate the second DMA buffer
    populateDmaBuffer();

    // Start the first DMA channel in the chain and both PWMs
    uint32_t pwm_mask = 0;

    pwmChannelAddStartList(&pwm_channel[0], &pwm_mask);
    pwmChannelAddStartList(&pwm_channel[1], &pwm_mask);

    // Build the DMA start mask
    uint32_t chan_mask = 0x01 << dma_channel[0];

    dma_start_channel_mask(chan_mask);
    pwmChannelStartList(pwm_mask);
}

void stopMusic(void)
{
    printf("stopMusic");
    // Disable DMAs and PWMs
    pwmChannelStop(&pwm_channel[0]);
    pwmChannelStop(&pwm_channel[1]);
    pwmChannelStop(&pwm_channel[0]);
    dma_channel_abort(dma_channel[0]);
    dma_channel_abort(dma_channel[1]);
    dma_channel_abort(dma_channel[0]);
}

void exitMusic(void)
{
    printf("exitMusic");
    // Stop music and unmount the file system
    stopMusic();
    current_state = off;
}

/*
 * populateCallback
 * buffer       Pointer to buffer to populate
 * len          Max number of 16 bit samples to copy into buffer
 * 
 * Write 16 bit stereo sound data to to the supplied buffer
 * callback function called from circular buffer class
 * 
 */
uint32_t populateCallback(int16_t* buffer, uint32_t len)
{
    uint32_t written = len;

    musicFileRead(&mf, buffer, len, &written);
    return written;
}

#include "resource.cpp"

/*
 * loadFile
 * filename     String containing name of music file to open
 * 
 * Returns true if music file was successfully opened and header read
 * 
 */
static bool loadFile(const char* filename)
{
    FIL fil;
    fil.data = outputmp3;
    fil.size = sizeof(outputmp3);
    fil.currentPosition = 0;
    
    mf.fil = fil;

    bool success = false;

    if (!musicFileCreate(&mf, filename, cache_buffer, CACHE_BUFFER))
    {
        printf("Cannot open file: %s\n", filename);
    }   
    else
    {
        success = true;
    }

    return success;
}
