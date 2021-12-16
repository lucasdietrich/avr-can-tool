#ifndef _SHELL_MODULES_H
#define _SHELL_MODULES_H


#ifdef __cplusplus
extern "C" {
#endif

#include <avrtos/kernel.h>

#include "app.h"

/*___________________________________________________________________________*/

typedef int8_t (*shell_module_handler_t) (char *cmd, uint8_t len);

struct shell_module {
        char name[8];
        uint8_t name_len;
        shell_module_handler_t handler;
        const char *help;
};

#define SHELL_MODULE(module_name, module_handler, help_p)       \
        {                                                       \
                .name = module_name,                            \
                .name_len = sizeof(module_name) - 1,            \
                .handler = module_handler,                      \
                .help = help_p                                 \
        }

shell_module_handler_t shell_get_module_handler(struct command *cmd, uint8_t *skip);

/*___________________________________________________________________________*/

int8_t shell_help_handler(char *cmd, uint8_t len);

/*___________________________________________________________________________*/


#ifdef __cplusplus
}
#endif

#endif