#include "monitor.h"

#include <avrtos/debug.h>

#include "parser.h"

struct k_signal sig_monitor = K_SIGNAL_INIT(sig_monitor);

void monitor_thread_entry(void *context)
{
        for (;;) {
                int8_t err = k_poll_signal(&sig_monitor, K_FOREVER);
                if (err == 0) {
                        const uint8_t val = sig_monitor.signal;
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

struct monitor_command
{
        char *what;
};

static const struct cmd_descr monitor_command_descr[] PROGMEM = {
        CMD_DESCR(struct monitor_command, what, CMD_TYPE_STRING)
};

PROGMEM_STRING(threads_s, "threads");
PROGMEM_STRING(canaries_s, "canaries");

int8_t monitor_shell_handler(char *cmd, uint8_t len)
{
        int8_t ret = -1;

        struct monitor_command data;
        ret = cmd_parse(cmd, len, monitor_command_descr,
                ARRAY_SIZE(monitor_command_descr), &data);
        if (ret > 0 && CMD_ARG_DEFINED(ret, 0)) {
                ret = 0;
                if (strcmp_P(data.what, threads_s) == 0) {
                        k_signal_raise(&sig_monitor, MONITOR_DUMP_THREADS);
                } else if (strcmp_P(data.what, canaries_s) == 0) {
                        k_signal_raise(&sig_monitor, MONITOR_DUMP_CANARIES);
                } else {
                        ret = -1;
                }
        }
        return ret;
}

