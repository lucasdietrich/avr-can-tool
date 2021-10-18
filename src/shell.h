#ifndef _SHELL_H
#define _SHELL_H

#include <avrtos/kernel.h>

/*___________________________________________________________________________*/

#define CMD_CANCELLED   -1
#define CMD_TOOLONG     -2

typedef struct
{
        struct qitem _reserved; /* void* */

        char buffer[64]; /* max buffer size : 127 */
        int8_t len;
} command;

/*___________________________________________________________________________*/

void shell_init(void);

void shell_thread(void *context);

void shell_handle_rx(const char rx);

/*___________________________________________________________________________*/

/* dispatcher */
void shell_dispatch_command(command *cmd);

/*___________________________________________________________________________*/

#endif