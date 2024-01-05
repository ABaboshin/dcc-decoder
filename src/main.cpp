#include <stdio.h>
#include <cstring>
#include <math.h>
#include <iostream>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "RP2040DccDecoder.h"
#include "Train.h"

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



  while (1)
  {
    // // std::cout  << "loop" << std::endl;
    // std::cout << "tick " << player.tick() << std::endl;
    // sleep_ms(22);
  }
}
