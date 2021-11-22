#ifndef _INIT_H
#define _INIT_H

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((naked, used, section(".init8"))) void os_init();

void hw_ll_init(void);

#ifdef __cplusplus
}
#endif

#endif