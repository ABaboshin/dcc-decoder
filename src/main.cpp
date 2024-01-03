#include <stdio.h>
#include "pico/stdlib.h"
#include "RP2040DccDecoder.h"
#include "Train.h"

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

  Train t(1, 3, 4);

  while (1)
  {

  }
}
