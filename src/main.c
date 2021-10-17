#include <avrtos/kernel.h>
#include <avrtos/debug.h>
#include <avrtos/misc/led.h>

#include "app.h"

struct k_signal sig_monitor = K_SIGNAL_INIT(sig_monitor);

int main()
{
        app_init();

        for (;;) {
                int8_t err = k_poll_signal(&sig_monitor, K_SECONDS(60));
                if (err == 0) {
                        const uint8_t val = sig_monitor.signal;
                        usart_u8(val);
                        if (TEST_BIT(val, MONITOR_DUMP_CANARIES)) {
                                dump_threads_canaries();
                        } else if (TEST_BIT(val, MONITOR_DUMP_THREADS)) {
                                k_thread_dump_all();
                        }
                        sig_monitor.flags = K_POLL_STATE_NOT_READY;
                } else if (err == -ETIMEOUT) {
                        dump_threads_canaries();
                }
        }
}