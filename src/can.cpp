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
        k_mem_slab_free(&can_msg_pool, (void *)msg);
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

