#include <stdio.h>
#include "RP2040DccDecoder.h"

int main()
{
  stdio_init_all();

  for (int i = 0; i <= 10; i++)
  {
    printf("Sleep on boot\n");
    sleep_ms(1000);
  }

  RP2040DccDecoder d(17, true);
  d.Start();

  while (1)
  {
  }
}
