#include "app.h"

#include <avrtos/debug.h>
#include <avrtos/misc/led.h>

#include "shell.h"

/*___________________________________________________________________________*/

// Application (Module)
#define K_MODULE 0

/*___________________________________________________________________________*/

void app_init(void)
{
    hw_init();
    usart_init();
    led_init();

    shell_init();

    k_thread_dump_all();

    __ASSERT_NOINTERRUPT();
}

/*___________________________________________________________________________*/

K_THREAD_DEFINE(app, thread_monitor, 0x50, K_PRIO_COOP(K_PRIO_MAX), NULL, 'M');

void thread_monitor(void *context)
{
    for (;;)
    {
        k_sleep(K_SECONDS(60));
        dump_threads_canaries();
    }
}