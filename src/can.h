#ifndef _CAN_H
#define _CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"

#define CAN_LOOPBACK_FLAG (1 << 0)

typedef struct 
{
        unsigned long id; /* uint32_t */
        uint8_t type;
        uint8_t buffer[8];
        uint8_t len;
} can_message;

typedef struct 
{
        void* _reserved;
        can_message msg;
} can_message_qi; /* qi = Queue Item */


typedef bool (*can_loopback_rule_t) (can_message *msg);

struct can_config
{
        union {
                uint8_t flags;
                struct {
                        uint8_t loopback : 1;
                };
        };
        can_loopback_rule_t loopback_rule;
        uint32_t masks[2];
        uint32_t filters[6];
        uint32_t speedset;
};

static inline void can_clear_message(can_message *msg)
{
        memset(msg, 0x00, sizeof(can_message));
}

void can_init(void);
void can_configure(struct can_config *cfg);
void can_rx_thread(void *context);
void can_tx_thread(void *context);
bool can_process_rx_message(can_message *buffer);

int8_t can_msg_alloc(can_message_qi **msg, k_timeout_t timeout);
void can_tx_msg_queue(can_message_qi *msg);
void can_msg_free(can_message_qi *msg);

uint8_t can_recv(can_message *msg);
uint8_t can_send(can_message *msg);

#define CAN_DIR_RX      1
#define CAN_DIR_TX      0
void can_show_message(can_message *msg, uint8_t dir);

bool can_loopback_rule(can_message *msg);

bool can_cfg_get_loopback(void);
void can_cfg_set_loopback(bool state);

#ifdef __cplusplus
}
#endif

#endif