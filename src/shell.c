#include "shell.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "app.h"
#include "shell_modules.h"

/*___________________________________________________________________________*/

#define K_MODULE K_MODULE_SHELL

/*___________________________________________________________________________*/

/* define count > 1 if some commands need time to be parse/processed */
K_MEM_SLAB_DEFINE(cmd_slab, sizeof(struct command), 1u);
K_FIFO_DEFINE(cmd_fifo);
K_THREAD_DEFINE(shell_thread, shell_entry, 0x100, K_PREEMPTIVE, NULL, '>');

void shell_entry(void *context)
{
        for (;;) {
                usart_print("\n# ");
                struct command *cmd = (struct command *)k_fifo_get(&cmd_fifo, K_FOREVER);

                __ASSERT_NOTNULL(cmd);

                switch (cmd->len) {
                case 0: /* LR only, do nothing */
                        break;
                case CMD_TOOLONG:
                {
                        printf_P(PSTR("\nToo long, max = 39"));
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

static void send_command(struct command **cmd)
{
        __ASSERT_NOTNULL(cmd);
        __ASSERT_NOTNULL(*cmd);

        k_fifo_put(&cmd_fifo, *(void **)cmd);
        *cmd = NULL;
}

inline void shell_handle_rx(const char rx)
{
        static struct command *cmd = NULL;

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
                cmd->len = 0;
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
                } else {
                        cmd->buffer[cmd->len++] = rx;
                }                

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

void shell_dispatch_command(struct command *cmd)
{
        int ret = -1;
        uint8_t skip = 0;

        /* execute module hanlder */
        shell_module_handler_t handler = shell_get_module_handler(cmd, &skip);
        if (handler != NULL) {
                const uint8_t len = skip >= cmd->len ? 0u : cmd->len - skip;
                ret = handler(cmd->buffer + skip, len);
                if (ret) {
                        printf_P(PSTR("\nMODULE ERROR: -%x"), -ret);
                }
        } else {
                printf_P(PSTR("\nNo such module!\n"));
        }
}

/*___________________________________________________________________________*/