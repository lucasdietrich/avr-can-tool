#ifndef _SHELL_CANIOT_H
#define _SHELL_CANIOT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int shell_caniot_handler(char *cmd, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* _SHELL_CANIOT_H */