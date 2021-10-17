#include "can.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <avrtos/kernel.h>

#include <mcp2515_can.h>

#include "parser.h"

#define K_MODULE K_MODULE_CAN
#define SPI_CS_PIN  10

mcp2515_can can(SPI_CS_PIN);

K_MUTEX_DEFINE(can_mutex_if);

K_THREAD_DEFINE(can_rx, can_rx_thread, 0x64, K_COOPERATIVE, NULL, 'R');
K_SIGNAL_DEFINE(can_sig_rx);

K_MEM_SLAB_DEFINE(can_msg_pool, sizeof(can_message_qi), 2u);
K_FIFO_DEFINE(can_tx_q);
K_THREAD_DEFINE(can_tx, can_tx_thread, 0x64, K_COOPERATIVE, NULL, 'T');

void can_init(void)
{
        while (CAN_OK != can.begin(CAN_500KBPS, MCP_8MHz)) {
                PRINT_PROGMEM_STRING(can_fail_msg, "can begin failed retry\n");
                k_sleep(K_MSEC(500));
        }

        /* interrupt when receiving a can message (falling on INT0) */
        EICRA |= 1 << ISC01;
        EICRA &= ~(1 << ISC00);
        EIMSK |= 1 << INT0;
}

ISR(INT0_vect)
{
        k_signal_raise(&can_sig_rx, 0u);
}

void can_rx_thread(void *context)
{
        static can_message msg;

        for (;;) {
                k_poll_signal(&can_sig_rx, K_FOREVER);
                /* K_SIGNAL_SET_UNREADY(&can_sig_rx); */
                can_sig_rx.flags = K_POLL_STATE_NOT_READY;

                while (can_recv(&msg, K_FOREVER) == 0) {
                        can_show_message(&msg, CAN_DIR_RX);
                }
        }
}

void can_tx_thread(void *context)
{
        can_message_qi *mem;

        for (;;) {
                mem = (can_message_qi *)k_fifo_get(&can_tx_q, K_FOREVER);
                can_message *const p_msg = &mem->msg;

                if (can_send(p_msg, K_FOREVER) == 0)
                        can_show_message(p_msg, CAN_DIR_TX);
                
                can_msg_free(mem);
        }
}

int8_t can_msg_alloc(can_message_qi **msg, k_timeout_t timeout)
{
        __ASSERT_NOTNULL(msg);

        return k_mem_slab_alloc(&can_msg_pool, (void **)msg, timeout);
}

void can_tx_msg_queue(can_message_qi *msg)
{
        __ASSERT_NOTNULL(msg);

        k_fifo_put(&can_tx_q, (struct qitem *)msg);
}

void can_msg_free(can_message_qi *msg)
{
        k_mem_slab_free(&can_msg_pool, (void*) msg);
}

void can_show_message(can_message *msg, uint8_t dir)
{       
        PROGMEM_STRING(s_rx, "RX ");
        PROGMEM_STRING(s_tx, "TX ");
        usart_print_p((dir == CAN_DIR_RX) ? s_rx : s_tx);

        usart_hex16((uint16_t)msg->id); /* assume STD id */
        usart_transmit(':');

        for (uint_fast8_t i = 0; i < MIN(msg->len, 8u); i++) {
                usart_transmit(' ');
                usart_hex(msg->buffer[i]);
        }
        usart_transmit('\n');
}

uint8_t can_recv(can_message *msg, k_timeout_t timeout)
{
        __ASSERT_NOTNULL(msg);

        uint8_t rc = k_mutex_lock(&can_mutex_if, timeout);
        if (rc == 0) {
                rc = -1;
                if (can.checkReceive() == CAN_MSGAVAIL)
                        rc = can.readMsgBufID(&msg->id, &msg->len, msg->buffer);
                
                k_mutex_unlock(&can_mutex_if);
        }
        return rc;
}

uint8_t can_send(can_message *msg, k_timeout_t timeout)
{
        __ASSERT_NOTNULL(msg);
        uint8_t rc = k_mutex_lock(&can_mutex_if, timeout);
        if (rc == 0) {
                rc = can.sendMsgBuf(msg->id, msg->type, msg->len, msg->buffer);
                
                k_mutex_unlock(&can_mutex_if);
        }
        return rc;
}

/*___________________________________________________________________________*/

struct can_command
{
        char *rxtx;
        unsigned int id;
        union 
        {
                struct {
                        unsigned int b0;
                        unsigned int b1;
                        unsigned int b2;
                        unsigned int b3;
                        unsigned int b4;
                        unsigned int b5;
                        unsigned int b6;
                        unsigned int b7;
                };
                unsigned int buffer[8];
        };
        
        
};

const struct cmd_descr can_command_descr[] PROGMEM = {
        CMD_DESCR(struct can_command, rxtx, CMD_TYPE_STRING),
        CMD_DESCR(struct can_command, id, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b0, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b1, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b2, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b3, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b4, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b5, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b6, CMD_TYPE_HEX),
        CMD_DESCR(struct can_command, b7, CMD_TYPE_HEX),
};

PROGMEM_STRING(rx_s, "rx");
PROGMEM_STRING(tx_s, "tx");

int8_t can_shell_handler(char *cmd, uint8_t len)
{
        int16_t args;
        int8_t ret;
        can_message_qi *p_msg = NULL;
        struct can_command data;

        /* parse arguments */
        args = cmd_parse(cmd, len, can_command_descr,
                ARRAY_SIZE(can_command_descr), &data);
        if (args <= 0 || !CMD_ARG_DEFINED(args, 0) || !CMD_ARG_DEFINED(args, 1)) {
                ret = (int8_t) args;
                goto exit;
        }

        /* allocate buffer */
        ret = can_msg_alloc(&p_msg, K_SECONDS(1));
        if (ret) {
                goto exit;
        }

        /* TX only supported now */
        ret = strcmp_P(data.rxtx, tx_s);
        if (ret) {
                ret = -1;
                goto exit;
        }

        /* set arbitration id */
        can_clear_message(&p_msg->msg);
        p_msg->msg.id = data.id;

        /* set buffer */
        for (uint_fast8_t i = 0; i < 8; i++) {
                if (CMD_ARG_DEFINED(args, 2 + i)) {
                        p_msg->msg.len++;
                        p_msg->msg.buffer[i] = (uint8_t) data.buffer[i];
                } else {
                        break;
                }
        }

        /* queue TX can message */
        can_tx_msg_queue(p_msg);

        return 0;

exit:
        can_msg_free(p_msg);
        return ret;        
}