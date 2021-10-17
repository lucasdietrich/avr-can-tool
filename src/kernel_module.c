#include "kernel_module.h"

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

PROGMEM_STRING(wait_s, "wait");
PROGMEM_STRING(random_s, "random");

K_PRNG_DEFINE_DEFAULT(prng);

int8_t kernel_shell_handler(char *cmd, uint8_t len)
{
        int8_t ret = -1;

        struct kernel_command data;
        ret = cmd_parse(cmd, len, kernel_command_descr,
                ARRAY_SIZE(kernel_command_descr), &data);
        if (ret > 0 && CMD_ARG_DEFINED(ret, 0)) {

                // usart_hex(ret);
                // usart_printl(cmd);

                /* kernel sleep */
                if (strcmp_P(data.what, wait_s) == 0) {
                        uint16_t delay_ms = 1000u;
                        if (CMD_ARG_DEFINED(ret, 1)) {
                                delay_ms = data.number;
                        }

                        k_sleep(K_MSEC(delay_ms));
                        ret = 0;
                } else if (strcmp_P(data.what, random_s) == 0) {
                        uint8_t buffer[10];
                        k_prng_get_buffer(&prng, buffer, sizeof(buffer));

                        usart_transmit('\n');
                        for (uint8_t i = 0; i < sizeof(buffer); i++) {
                                usart_hex(buffer[i]);
                                usart_transmit(' ');
                        }
                        ret = 0;
                }
        }
        return ret;
}

