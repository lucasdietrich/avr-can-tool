#ifndef _MONITOR_H
#define _MONITOR_H

#include <avrtos/kernel.h>

#include "app.h"

/*___________________________________________________________________________*/

#define MONITOR_DUMP_CANARIES   (1 << 0)
#define MONITOR_DUMP_THREADS    (1 << 1)

/*___________________________________________________________________________*/

void monitor_thread_entry(void *context);

int8_t monitor_shell_handler(char *cmd, uint8_t len);

/*___________________________________________________________________________*/

#endif