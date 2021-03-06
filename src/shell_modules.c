#include "shell_modules.h"

#include "parser.h"
#include "shell_can.h"
#include "kernel_module.h"
#include "shell_caniot.h"

/*___________________________________________________________________________*/

#define K_MODULE K_MODULE_SHELL

/*___________________________________________________________________________*/

// static const char can_help[] PROGMEM =
//         "- can tx {id} {b1} {b2} {b3} {b4} {b5} {b6} {b7}\n"
//         "\t> can tx 20F AA BB CC DD\n";

// static const char kernel_help[] PROGMEM = "- wait {delay_ms}\n"
//         "\t> kernel wait 1000\n"
//         "- prng\n"
//         "\t> kernel prng\n"
//         "- uptime\n"
//         "\t> kernel uptime\n"
//         "- canaries\n"
//         "\t> kernel canaries\n"
//         "- threads\n"
//         "\t> kernel threads\n";

#define SHELL_SHORTCUT 1

static const struct shell_module modules[] PROGMEM = {
        SHELL_MODULE("can", can_shell_handler, NULL),
        SHELL_MODULE("kernel", kernel_shell_handler, NULL),
        SHELL_MODULE("caniot", shell_caniot_handler, NULL),
        SHELL_MODULE("help", shell_help_handler, NULL),
#if SHELL_SHORTCUT
        SHELL_MODULE("k", kernel_shell_handler, NULL), /* k is a shortcut for kernel */
        SHELL_MODULE("ct", shell_caniot_handler, NULL), /* "ct" is a shortcut for caniot */
#endif /* SHELL_SHORTCUT */
};

shell_module_handler_t shell_get_module_handler(struct command *cmd, uint8_t *skip) {
        __ASSERT_NOTNULL(cmd);

        shell_module_handler_t handler = NULL;

        for(uint8_t i = 0; i < ARRAY_SIZE(modules); i++) {
                const unsigned char name_len = pgm_read_byte(&modules[i].name_len);
                if ((strncmp_P(cmd->buffer, modules[i].name, name_len) == 0) &&
                    ((cmd->len == name_len) ||
                     (cmd->buffer[name_len] == ' '))) {
                        handler = pgm_read_ptr(&modules[i].handler);
                        
                        /* tell how many to skip to get subcommand if any*/
                        *skip = name_len + 1u;
                        break;
                }
        }
        return handler;
}

/*___________________________________________________________________________*/

struct shell_help_module
{
        char *module_name;
};

static const struct cmd_descr shell_help_module[] PROGMEM = {
        CMD_DESCR(struct shell_help_module, module_name, CMD_TYPE_STRING),
};

int shell_help_handler(char *cmd, uint8_t len)
{
        const struct shell_module *module = NULL;

        /* get the module for which to display help */
        struct shell_help_module data;
        if (cmd_parse(cmd, len, shell_help_module,
                ARRAY_SIZE(shell_help_module), &data) > 0) {
                for (uint8_t i = 0; i < ARRAY_SIZE(modules); i++) {
                        if (strcmp_P(data.module_name, modules[i].name) == 0) {
                                module = &modules[i];
                                break;
                        }
                }
        }

        if (module == NULL) {
                /* get modules list */
                for (uint8_t i = 0; i < ARRAY_SIZE(modules); i++) {
                        printf_P(PSTR("\n\t"));
                        usart_print_p(modules[i].name);
                }
        } else {
                /* get help for a specific module */
                const char *descr_p = pgm_read_ptr(&module->help);
                usart_transmit('\n');
                usart_print_p(descr_p == NULL
                        ? PSTR("Module has no description")
                        : descr_p);
        }

        return 0;
}