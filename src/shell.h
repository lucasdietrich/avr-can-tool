#ifndef _SHELL_H
#define _SHELL_H


#ifdef __cplusplus
extern "C" {
#endif

#include <avrtos/kernel.h>

/*___________________________________________________________________________*/

#define CMD_CANCELLED   -1
#define CMD_TOOLONG     -2

#define MAX_CMD_LEN     sizeof("can tx 1FFFFFFF FF FF FF FF FF FF FF FF")

struct command
{
        struct qitem _reserved; /* void* */

        char buffer[MAX_CMD_LEN]; /* max buffer size : 127 */
        int8_t len;
};

/*___________________________________________________________________________*/

void shell_init(void);

void shell_entry(void *context);

void shell_handle_rx(const char rx);

/*___________________________________________________________________________*/

/* dispatcher */
void shell_dispatch_command(struct command *cmd);

/*___________________________________________________________________________*/

#ifdef __cplusplus
}
#endif

#endif