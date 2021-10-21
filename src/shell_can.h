#ifndef _SHELL_CAN_H
#define _SHELL_CAN_H


#ifdef __cplusplus
extern "C" {
#endif

#include "can.h"

int8_t can_shell_handler(char *cmd, uint8_t len);


#ifdef __cplusplus
}
#endif

#endif