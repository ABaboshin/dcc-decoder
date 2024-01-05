#include <stdio.h>
#include <cstring>
#include <math.h>
#include <iostream>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "RP2040DccDecoder.h"
#include "Train.h"
#include "MP3Player.h"
#include "Adafruit_MP3.h"

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void writeDacs(int16_t l, int16_t r){
  std::cout << "write dac " << l  << " " << r << std::endl;
  uint16_t val = map(l, -32768, 32767, 0, 4095);
  pwm_set_gpio_level(10, val);

// #if defined(__SAMD51__) // feather/metro m4
//   analogWrite(A0, val);
// #elif defined(__MK66FX1M0__) || defined(__MK20DX256__) // teensy 3.6
//   analogWrite(A21, val);
// #endif
}

uint8_t *currentPtr;
uint32_t thisManyBytesLeft;

int getMoreData(uint8_t *writeHere, int thisManyBytes){
  std::cout << "getMoreData " << writeHere  << " " << thisManyBytes << std::endl;
  int toWrite = fmin(thisManyBytesLeft, thisManyBytes);
  memcpy(writeHere, currentPtr, toWrite);
  currentPtr += toWrite;
  thisManyBytesLeft -= toWrite;
  std::cout << "getMoreData " << writeHere  << " " << thisManyBytes << " " << toWrite << std::endl;
  return toWrite;
}

#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

Adafruit_MP3 player;

static void alarm_irq(void);

static void alarm_in_us(uint32_t delay_us) {
    // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
    hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);
    // Set irq handler for alarm irq
    irq_set_exclusive_handler(ALARM_IRQ, alarm_irq);
    // Enable the alarm irq
    irq_set_enabled(ALARM_IRQ, true);
    // Enable interrupt in block and at processor

    // Alarm is only 32 bits so if trying to delay more
    // than that need to be careful and keep track of the upper
    // bits
    uint64_t target = timer_hw->timerawl + delay_us;

    // Write the lower 32 bits of the target time to the alarm which
    // will arm it
    timer_hw->alarm[ALARM_NUM] = (uint32_t) target;
}

static void alarm_irq(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);

    // Assume alarm 0 has fired
    player.tick();
    MP3_Handler();
    alarm_in_us(1000000 / 16000);
}



#include "resource.cpp"

int main()
{
  stdio_init_all();

  for (int i = 0; i <= 10; i++)
  {
    printf("Sleep on boot\n");
    sleep_ms(1000);
  }

  // RP2040DccDecoder d(17, true);
  // d.Start();

  // Train t(1, 3, 4);

  // MP3Player p(10, 11);
  // p.Play();

  

  PwmData data;
  data.Pin = 10;

std::cout  << "pin init 1" << std::endl;
  gpio_init(data.Pin);
std::cout  << "pin init 2" << std::endl;
    auto config = pwm_get_default_config();
    gpio_set_function(data.Pin, GPIO_FUNC_PWM);
    data.PinSlice = pwm_gpio_to_slice_num(data.Pin);
std::cout  << "pin init 3" << std::endl;
    pwm_config_set_clkdiv(&config, 1.0f); 
    pwm_config_set_wrap(&config, 4082); 
std::cout  << "pin init 4" << std::endl;
    pwm_init(data.PinSlice, &config, false);
std::cout  << "pin init 5" << std::endl;
    // pwm_set_gpio_level(fwdPin, 4999 / speed * 128);

    currentPtr = (uint8_t*)outputmp3;
  thisManyBytesLeft = sizeof(outputmp3);

  

  std::cout  << "pin init 6" << std::endl;

  std::cout << player.begin() << std::endl;

  //do this when there are samples ready
  player.setSampleReadyCallback(writeDacs);

  //do this when more data is required
  player.setBufferCallback(getMoreData);

std::cout  << "play" << std::endl;

  player.play();

  alarm_in_us(1000000 / 16000);


  while (1)
  {
    // // std::cout  << "loop" << std::endl;
    // std::cout << "tick " << player.tick() << std::endl;
    // sleep_ms(22);
  }
}
