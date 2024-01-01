#include <stdio.h>
#include "RP2040DccDecoder.h"

#define DCC_INPUT_PIN 17

int main()
{
  stdio_init_all();

  for (int i = 0; i <= 10; i++)
  {
    printf("Sleep on boot\n");
    sleep_ms(1000);
  }

  RP2040DccDecoder d;

  while (1)
  {
  }
}
