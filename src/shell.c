#include "shell.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "app.h"
#include "parser.h"

/*___________________________________________________________________________*/

#define K_MODULE K_MODULE_SHELL

/*___________________________________________________________________________*/

/* define count > 1 if some commands need time to be parse/processed */
K_MEM_SLAB_DEFINE(cmd_slab, sizeof(command), 2u);
K_FIFO_DEFINE(cmd_fifo);
K_THREAD_DEFINE(shell, shell_thread, 0x100,
        K_PRIO_PREEMPT(K_PRIO_LOW), NULL, '>');

void shell_thread(void *context)
{
        for (;;) {
                usart_print("\n> ");
                command *cmd = (command *)k_fifo_get(&cmd_fifo, K_FOREVER);
                switch (cmd->len) {
                case 0: /* LR only, do nothing */
                        break;
                case CMD_CANCELLED:
                {
                        static const char cancelled[] PROGMEM = "\nCancelled";
                        usart_print_p(cancelled);
                }
                break;
                case CMD_TOOLONG:
                {
                        static const char toolong[] PROGMEM =
                                "\nToo long, max = 64";
                        usart_print_p(toolong);
                }
                break;
                default:
                        if (cmd->len >= 0)
                                shell_dispatch_command(cmd);
                        break;
                }
                k_mem_slab_free(&cmd_slab, cmd);
        }
}

void shell_init(void)
{
        /* set USART0 RX interrupt */
        SET_BIT(UCSR0B, 1 << RXCIE0);
}

static void send_command(command **command)
{
        __ASSERT_NOTNULL(*command);
        k_fifo_put(&cmd_fifo, *(void **)command);
        *command = NULL;
}

inline void shell_handle_rx(const char rx)
{
        static command *cmd = NULL;

        if (cmd == NULL) {
                if (k_mem_slab_alloc(&cmd_slab, (void **)&cmd, K_NO_WAIT) != 0) {
                        __ASSERT_NULL(cmd);
                        usart_transmit('!'); /* todo remove, do fault */
                        return;
                }

                /* reset len as the buffer is not cleared when freed */
                cmd->len = 0;
        }

        switch (rx) {
        case 0x1A: /* Ctrl + Z -> drop */
                cmd->len = CMD_CANCELLED;
                send_command(&cmd);
                break;

        case '\n': /* process the packet */
                cmd->buffer[cmd->len] = '\0';
                send_command(&cmd);
                break;

        case 0x08: /* delete last char */
                if (cmd->len != 0) {
                        cmd->len--;
                        usart_transmit(rx);
                }
                break;

        default:
                if (cmd->len == sizeof(cmd->buffer) - 1u) {
                        cmd->len = CMD_TOOLONG;
                        send_command(&cmd);
                }

                cmd->buffer[cmd->len++] = rx;

                /* notify io stream
                 * unoptimal, move console stream in another thread
                 */
                usart_transmit(rx);
                break;
        }
}

ISR(USART_RX_vect)
{
        const char rx = UDR0;
        shell_handle_rx(rx);
}

/*___________________________________________________________________________*/

static const struct shell_module modules[] PROGMEM = {
        SHELL_MODULE("can", can_shell_handler),
        SHELL_MODULE("caniot", NULL),
        SHELL_MODULE("kernel", kernel_shell_handler),
        SHELL_MODULE("monitor", monitor_shell_handler),
};

static shell_module_handler_t find_module_handler(command *cmd, uint8_t *skip) {
        __ASSERT_NOTNULL(cmd);

        shell_module_handler_t handler = NULL;

        for(uint8_t i = 0; i < ARRAY_SIZE(modules); i++) {
                const char name_len = pgm_read_byte(&modules[i].name_len);
                if (strncmp_P(cmd->buffer, modules[i].name, name_len) == 0) {
                        handler = pgm_read_ptr(&modules[i].handler);
                        *skip = name_len + 1u;
                        break;
                }
        }
        return handler;
}


PROGMEM_STRING(dispatch_notfound, "\nModule not found !\n");
PROGMEM_STRING(dispatch_failed, "\nFailed to execute module handler err = ");

void shell_dispatch_command(command *cmd)
{
        int8_t ret = -1;
        uint8_t skip = 0;

        shell_module_handler_t handler = find_module_handler(cmd, &skip);
        if (handler != NULL) {
                const uint8_t len = skip >= cmd->len ? 0u : cmd->len - skip;
                ret = handler(cmd->buffer + skip, len);
                if (ret) {
                        usart_print_p(dispatch_failed);
                        usart_hex(ret);
                        usart_transmit('\n');
                }
        } else {
                usart_print_p(dispatch_notfound);
        }
}
