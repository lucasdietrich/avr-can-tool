#ifndef _CANTOOL_CANIOT_CONTROLLER_H
#define _CANTOOL_CANIOT_CONTROLLER_H

#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

int caniot_controller_process_frame(can_message *msg);

#ifdef __cplusplus
}
#endif

#endif /* _CANTOOL_CANIOT_H */