#include <stdio.h>
#include "pico/stdlib.h"
#include "RP2040DccDecoder.h"
#include "Train.h"
#include "MP3Player.h"

extern "C" 
{
  int mainsound();
}

int main()
{
  stdio_init_all();

  for (int i = 0; i <= 10; i++)
  {
    printf("Sleep on boot\n");
    sleep_ms(1000);
  }

  mainsound();

  // RP2040DccDecoder d(17, true);
  // d.Start();

  // Train t(1, 3, 4);

  // MP3Player p(10, 11);
  // p.Play();

  // while (1)
  // {

  // }
}
