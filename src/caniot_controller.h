#ifndef _CANTOOL_CANIOT_CONTROLLER_H
#define _CANTOOL_CANIOT_CONTROLLER_H

#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

union deviceid;

extern struct k_msgq caniot_frame_q;

int queue_caniot_rxframe(const can_message *p_msg);

int request_telemetry(union deviceid did, uint8_t ep, k_timeout_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* _CANTOOL_CANIOT_H */