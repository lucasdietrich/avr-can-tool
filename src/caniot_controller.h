#ifndef _CANTOOL_CANIOT_CONTROLLER_H
#define _CANTOOL_CANIOT_CONTROLLER_H

#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

union deviceid;

int process_caniot_frame(can_message *msg);

int request_telemetry(union deviceid did, uint8_t ep, k_timeout_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* _CANTOOL_CANIOT_H */