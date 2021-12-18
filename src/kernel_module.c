#include "kernel_module.h"

#include <avrtos/debug.h>
#include "parser.h"

struct kernel_command
{
        char *what;
        unsigned int number;
};

static const struct cmd_descr kernel_command_descr[] PROGMEM = {
        CMD_DESCR(struct kernel_command, what, CMD_TYPE_STRING),
        CMD_DESCR(struct kernel_command, number, CMD_TYPE_NUMBER)
};

K_PRNG_DEFINE_DEFAULT(prng);

int kernel_shell_handler(char *cmd, uint8_t len)
{
        int ret = -1;

        struct kernel_command data;
        ret = cmd_parse(cmd, len, kernel_command_descr,
                ARRAY_SIZE(kernel_command_descr), &data);
        if (ret > 0 && CMD_ARG_DEFINED(ret, 0)) {
                
                /* kernel sleep */
                if (strcmp_P(data.what, PSTR("wait")) == 0) {
                        uint16_t delay_ms = 1000u;
                        if (CMD_ARG_DEFINED(ret, 1)) {
                                delay_ms = data.number;
                        }

                        k_sleep(K_MSEC(delay_ms));
                        ret = 0;
                
                /* random number */
                } else if (strcmp_P(data.what, PSTR("prng")) == 0) {
                        uint8_t buffer[10];
                        k_prng_get_buffer(&prng, buffer, sizeof(buffer));
                        printf_P(PSTR(" : "));
                        for (uint8_t i = 0; i < sizeof(buffer); i++) {
                                printf_P(PSTR("%02hhx "), buffer[i]);
                        }
                        ret = 0;
                
                /* uptime */
                } else if (strcmp_P(data.what, PSTR("uptime")) == 0) {
                        show_uptime();
                        ret = 0;

                /* threads canaries */
                } else if (strcmp_P(data.what, PSTR("canaries")) == 0) {
                        printf_P(PSTR("\n"));
                        dump_threads_canaries();
                        ret = 0;

                /* threads */
                } else if (strcmp_P(data.what, PSTR("threads")) == 0) {
                        printf_P(PSTR("\n"));
                        k_thread_dump_all();
                        ret = 0;
                }
        }
        return ret;
}

void show_uptime(void)
{
        struct timespec ts;
        k_timespec_get(&ts);

        uint32_t seconds = ts.tv_sec;
        uint32_t minutes = seconds / 60;
        uint32_t hours = minutes / 60;

        printf_P(PSTR(" : %02lu:%02hhu:%02hhu [%lu.%03u s]"),
                 hours, (uint8_t)(minutes % 60), (uint8_t)(seconds % 60),
                 ts.tv_sec, ts.tv_msec);
}

