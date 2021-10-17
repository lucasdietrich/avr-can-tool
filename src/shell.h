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

typedef int8_t (*shell_module_handler_t) (char *cmd, uint8_t len);

struct shell_module {
        char name[8];
        uint8_t name_len;
        shell_module_handler_t handler;
};

#define SHELL_MODULE(module_name, module_handler)       \
        {                                               \
                .name = module_name,                    \
                .name_len = sizeof(module_name) - 1,    \
                .handler = module_handler               \
        }

/*___________________________________________________________________________*/

#endif