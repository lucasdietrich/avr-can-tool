#ifndef _KERNEL_MODULE_H
#define _KERNEL_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <avrtos/kernel.h>

#include "app.h"

/*___________________________________________________________________________*/

void show_uptime(void);

int kernel_shell_handler(char *cmd, uint8_t len);

/*___________________________________________________________________________*/

#ifdef __cplusplus
}
#endif

#endif