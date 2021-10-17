#include "shell.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "can.h"

#include "app.h"

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
                switch (cmd->cursor) {
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
                        if (cmd->cursor >= 0)
                                shell_process_command(cmd);
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

                /* reset cursor as the buffer is not cleared when freed */
                cmd->cursor = 0;
        }

        switch (rx) {
        case 0x1A: /* Ctrl + Z -> drop */
                cmd->cursor = CMD_CANCELLED;
                send_command(&cmd);
                break;

        case '\n': /* process the packet */
                cmd->buffer[cmd->cursor] = '\0';
                send_command(&cmd);
                break;

        case 0x08: /* delete last char */
                if (cmd->cursor != 0) {
                        cmd->cursor--;
                        usart_transmit(rx);
                }
                break;

        default:
                if (cmd->cursor == sizeof(cmd->buffer) - 1u) {
                        cmd->cursor = CMD_TOOLONG;
                        send_command(&cmd);
                }

                cmd->buffer[cmd->cursor++] = rx;

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

void shell_process_command(command *cmd)
{
        __ASSERT_NOTNULL(cmd);

        shell_parse_command(cmd);
}

extern struct k_signal sig_monitor;

int8_t shell_parse_command(command *cmd)
{
        char buffer[128];

        sprintf(buffer, "\nReceived command [%d] : %s",
                cmd->cursor, cmd->buffer);
        usart_print(buffer);

        /* demo signal */
        if (strncmp(cmd->buffer, "monitor canaries", sizeof("monitor canaries")) == 0) {
                k_signal_raise(&sig_monitor, MONITOR_DUMP_CANARIES);
        } else if (strncmp(cmd->buffer, "monitor threads", sizeof("monitor threads")) == 0) {
                k_signal_raise(&sig_monitor, MONITOR_DUMP_THREADS);
        } else if (strncmp(cmd->buffer, "can send", sizeof("can send")) == 0) {
                can_message_qi *p_msg = NULL;
                int8_t err = can_msg_alloc(&p_msg, K_SECONDS(1));
                if (err == 0) {
                        p_msg->msg.id = 0xADECu;
                        p_msg->msg.len = 2u;
                        p_msg->msg.type = 1u; /* STD = 0, EXT = 1 */
                        p_msg->msg.buffer[0] = 0x12u;
                        p_msg->msg.buffer[0] = 0x34u;
                        can_tx_msg_queue(p_msg);
                }
        } else if (strncmp(cmd->buffer, "wait", sizeof("wait")) == 0) {
                k_sleep(K_SECONDS(1));
        }

        return 0;
}