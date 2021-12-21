#ifndef _APP_H
#define _APP_H

#ifdef __cplusplus
extern "C" {
#endif

/*___________________________________________________________________________*/

#include <avrtos/kernel.h>

#include "defines.h"

/*___________________________________________________________________________*/

#include "init.h"
#include "shell.h"
#include "can.h"


/*___________________________________________________________________________*/

void app_init(void);

/*___________________________________________________________________________*/

#ifdef __cplusplus
}
#endif

#endif /* _APP_H */