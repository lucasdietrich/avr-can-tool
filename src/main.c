#include <avrtos/kernel.h>
#include <avrtos/misc/led.h>

#include "app.h"

int main()
{
  app_init();

  for (;;)
  {
    k_sleep(K_SECONDS(1));
    led_on();
    k_sleep(K_SECONDS(1));
    led_off();
  }
}