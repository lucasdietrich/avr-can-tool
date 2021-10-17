#ifndef _KERNEL_MODULE_H
#define _KERNEL_MODULE_H

#include <avrtos/kernel.h>

#include "app.h"

/*___________________________________________________________________________*/

int8_t kernel_shell_handler(char *cmd, uint8_t len);

/*___________________________________________________________________________*/

#endif