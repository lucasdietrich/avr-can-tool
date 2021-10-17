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
        int8_t cursor;
} command;

void shell_init(void);

inline void shell_handle_rx(const char rx);

int8_t shell_parse_command(command *cmd);

void shell_thread(void *context);

/*___________________________________________________________________________*/

#endif