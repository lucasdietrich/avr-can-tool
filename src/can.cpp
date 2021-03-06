#include "can.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include <avrtos/kernel.h>

#include <mcp2515_can.h>
#include <mcp2515_can_dfs.h>

#include "parser.h"
#include "caniot_controller.h"

#define K_MODULE K_MODULE_CAN
#define SPI_CS_PIN  10

mcp2515_can can(SPI_CS_PIN);

K_MUTEX_DEFINE(can_mutex_if);

K_THREAD_DEFINE(can_rx_thread, can_rx_entry, 0x64, K_COOPERATIVE, NULL, 'R');
K_SIGNAL_DEFINE(can_sig_rx);

/* 2 should be enough : CMD + RX */
K_MEM_SLAB_DEFINE(can_msg_pool, sizeof(can_message_qi), 2u);
K_FIFO_DEFINE(can_tx_q);
K_THREAD_DEFINE(can_tx_thread, can_tx_entry, 0x64, K_COOPERATIVE, NULL, 'T');

#ifdef CONFIG_CAN_CONFIG_FLAGS
#       define CAN_CONFIG_FLAGS CONFIG_CAN_CONFIG_FLAGS
#else 
#       define CAN_CONFIG_FLAGS CAN_RX_FLAG
#endif

#ifdef CONFIG_CAN_INT
#       define CAN_INT CONFIG_CAN_INT
#else 
#       define CAN_INT 0
#endif

#ifdef CONFIG_CAN_CLOCKSET_16MHZ
#       define CAN_CLOCKSET MCP_16MHz
#else 
#       define CAN_CLOCKSET MCP_8MHz
#endif

static struct can_config config = {
        {
                .flags = CAN_CONFIG_FLAGS
        },
        .loopback_rule = can_loopback_rule,
        .masks = {0, 0},
        .filters = {0, 0, 0, 0, 0, 0},
        .speedset = CAN_500KBPS,
};

void can_init(void)
{
        can_configure(&config);

        /* interrupt when receiving a can message */
#if CAN_INT == 0
        /* falling on INT0 */
        EICRA |= 1 << ISC01;
        EICRA &= ~(1 << ISC00);
        EIMSK |= 1 << INT0;
#elif CAN_INT == 1
        // falling on INT1
        EICRA |= 1 << ISC11;
        EICRA &= ~(1 << ISC10);
        EIMSK |= 1 << INT1;
#else
#       error "Invalid CAN_CONFIG_INT value"
#endif
}

void can_configure(struct can_config *cfg)
{
        k_mutex_lock(&can_mutex_if, K_FOREVER);

        while (CAN_OK != can.begin(cfg->speedset, CAN_CLOCKSET)) {
                printf_P(PSTR("can begin failed retry\n"));
                k_sleep(K_MSEC(500));
        }

        can.init_Mask(0, CAN_EXTID, cfg->masks[0]);
        if (cfg->masks[0]) {
                can.init_Filt(0, CAN_EXTID, cfg->filters[0]);
                can.init_Filt(1, CAN_EXTID, cfg->filters[1]);
        }

        can.init_Mask(1, CAN_EXTID, cfg->masks[1]);
        if (cfg->masks[0]) {
                can.init_Filt(2, CAN_EXTID, cfg->filters[2]);
                can.init_Filt(3, CAN_EXTID, cfg->filters[3]);
                can.init_Filt(4, CAN_EXTID, cfg->filters[4]);
                can.init_Filt(5, CAN_EXTID, cfg->filters[5]);
        }

        k_mutex_unlock(&can_mutex_if);
}

static uint8_t can_recv(can_message *msg)
{
        __ASSERT_NOTNULL(msg);

        uint8_t rc = k_mutex_lock(&can_mutex_if, K_MSEC(100));
        if (rc == 0) {
                rc = -1;
                if (can.checkReceive() == CAN_MSGAVAIL) {
                        uint8_t isext, rtr;
                        rc = can.readMsgBufID(can.readRxTxStatus(),
                                              (unsigned long *)&msg->id, &isext, &rtr,
                                              &msg->len, msg->buf);
                        if (rc == 0) {
                                msg->isext = isext ? CAN_EXTID : CAN_STDID;
                                msg->rtr = rtr ? 1 : 0;
                        }
                }
                k_mutex_unlock(&can_mutex_if);
        }
        return rc;
}

static uint8_t can_send(can_message *msg)
{
        __ASSERT_NOTNULL(msg);
        uint8_t rc = k_mutex_lock(&can_mutex_if, K_MSEC(100));
        if (rc == 0) {
                rc = can.sendMsgBuf(msg->id, msg->isext, msg->rtr, msg->len,
                                    msg->buf, true);

                k_mutex_unlock(&can_mutex_if);
        }
        return rc;
}

#if CAN_INT == 0
ISR(INT0_vect)
#else
ISR(INT1_vect)
#endif /* CAN_INT */
{
        if (config.rxint) {
                usart_transmit('*');
        }


        k_signal_raise(&can_sig_rx, 0u);
}

void can_rx_entry(void *context)
{
        static can_message msg;

        for (;;) {
                k_poll_signal(&can_sig_rx, K_FOREVER);
                can_sig_rx.flags = K_POLL_STATE_NOT_READY;

                while (can_process_rx_message(&msg)) {
                        /* yield to allow tx thread to process
                         * loopback packet if any */
                        k_yield();
                }
        }
}

bool can_process_rx_message(can_message *buffer)
{
        can_message *p_msg = buffer;
        can_message_qi *p_msg_qi = NULL;

        if (config.loopback && (config.loopback_rule != NULL)) {
                if (can_msg_alloc(&p_msg_qi, K_MSEC(100u)) == 0) {
                        p_msg = &p_msg_qi->msg;
                }
        }

        if (can_recv(p_msg) == 0) {
                /* TX threads is cooperative, p_msg_qi will be deallocated
                 * only after the frame is processed
                 */


                /* show the received packet */
                if (config.rx) {
                        can_show_message(p_msg, CAN_DIR_RX);
			printf_P(PSTR("\n"));
                }

                /* if the packet should be processed as a caniot packet */
#if defined(CONFIG_CANIOT_LIB)
		// if caniot paquet, queue it
		int ret = queue_caniot_rxframe(p_msg);
		if (ret != 0) {
			printf_P(PSTR("caniot queue failed : -%d\n"), -ret);
		}
#endif

                /* if loopback and allocation succeeded */
                if (p_msg_qi != NULL) {
                        __ASSERT_NOTNULL(config.loopback_rule);

                        /* loopback rule is necessarily not null here */
                        if (config.loopback_rule(p_msg)) {
                                can_tx_msg_queue(p_msg_qi);
                        } else {
                                can_msg_free(p_msg_qi);
                        }
                }

                return true;
        }

        can_msg_free(p_msg_qi);
        return false;
}

void can_tx_entry(void *context)
{
	uint8_t ret;
        can_message_qi *mem;

        for (;;) {
                mem = (can_message_qi *)k_fifo_get(&can_tx_q, K_FOREVER);
                can_message *const p_msg = &mem->msg;

		ret = can_send(p_msg);
                if (ret == 0) {
			can_show_message(p_msg, CAN_DIR_TX);
		} else {
			printf_P(PSTR("Err sending CAN message : %d\n"), ret);
		}

                can_msg_free(mem);

                /* yield to allow rx thread to process
                * an incoming message if any */
                k_yield();
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

// show can_message structure 
void can_show_message(can_message *msg, uint8_t dir)
{
	__ASSERT_NOTNULL(msg);

	printf_P(PSTR("[ %s "), dir == CAN_DIR_RX ? "RX" : "TX");

	if (msg->isext) {
		printf_P(PSTR("%08lx"), msg->id);
	} else {
		printf_P(PSTR("%03lx"), msg->id);
	}

	// show the data and length if not rtr
	if (!msg->rtr) {
		if (msg->len > 0) {
			printf_P(PSTR(" /"), msg->len);

			for (uint8_t i = 0; i < msg->len; i++) {
				printf_P(PSTR(" %02hhx"), msg->buf[i]);
			}
		}
	} else {
		// show len if rtr
		printf_P(PSTR(" RTR %02x"), msg->len);
	}

	printf_P(PSTR(" ] "));
}

/*___________________________________________________________________________*/

bool can_loopback_rule(can_message *msg)
{
        // static uint32_t id = 0u;

        static uint32_t count = 0u;
        if (++count > 10u) {
                count = 0u;
                return false;
        }
        return true;
}

/*___________________________________________________________________________*/

bool can_cfg_get_loopback(void)
{
        return (bool)config.loopback;
}

void can_cfg_set_loopback(bool state)
{
        config.loopback = state ? 1u : 0u;
}

bool can_cfg_get_rx(void)
{
        return (bool)config.rx;
}

void can_cfg_set_rx(bool state)
{
        config.rx = state ? 1u : 0u;
}

bool can_cfg_get_int(void)
{
        return (bool)config.rxint;
}

void can_cfg_set_int(bool state)
{
        config.rxint = state ? 1u : 0u;
}

