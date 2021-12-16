#ifndef _CAN_H
#define _CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app.h"

#define CAN_LOOPBACK_FLAG (1 << 0)
#define CAN_RX_FLAG (1 << 1)
#define CAN_INT_FLAG (1 << 2)

typedef struct 
{
        union {
                uint32_t std: 11;
                uint32_t ext: 29;
                uint32_t id;
        };
        uint8_t isext: 1;
        uint8_t rtr: 1;

        struct {
                uint8_t buf[8];
                uint8_t len;
        };
} __attribute__((packed)) can_message;

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
                        uint8_t rx : 1;
                        uint8_t rxint : 1;
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

#define CAN_DIR_RX      1
#define CAN_DIR_TX      0
void can_show_message(can_message *msg, uint8_t dir);

bool can_loopback_rule(can_message *msg);

bool can_cfg_get_loopback(void);
void can_cfg_set_loopback(bool state);

bool can_cfg_get_rx(void);
void can_cfg_set_rx(bool state);

bool can_cfg_get_int(void);
void can_cfg_set_int(bool state);

#ifdef __cplusplus
}
#endif

#endif