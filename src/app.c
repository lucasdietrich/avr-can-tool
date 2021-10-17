#include "app.h"

#include <avrtos/debug.h>
#include <avrtos/misc/led.h>

#include "shell.h"
#include "can.h"

#include <Arduino.h>

/*___________________________________________________________________________*/

// Application (Module)
#define K_MODULE 0

/*___________________________________________________________________________*/

void app_init(void)
{
        hw_ll_init();
        usart_init();
        led_init();
        can_init();
        shell_init();

        k_thread_dump_all();

        __ASSERT_NOINTERRUPT();
}
