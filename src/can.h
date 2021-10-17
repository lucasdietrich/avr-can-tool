#ifndef _CAN_H
#define _CAN_H

#include "app.h"

#ifdef __cplusplus
extern "C" {
#endif

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

inline void can_clear_message(can_message *msg)
{
        memset(msg, 0x00, sizeof(can_message));
}

void can_init(void);
void can_rx_thread(void *context);
void can_tx_thread(void *context);

int8_t can_msg_alloc(can_message_qi **msg, k_timeout_t timeout);
void can_tx_msg_queue(can_message_qi *msg);
void can_msg_free(can_message_qi *msg);

uint8_t can_recv(can_message *msg, k_timeout_t timeout);
uint8_t can_send(can_message *msg, k_timeout_t timeout);

#define CAN_DIR_RX      1
#define CAN_DIR_TX      0
void can_show_message(can_message *msg, uint8_t dir);

#ifdef __cplusplus
}
#endif

#endif