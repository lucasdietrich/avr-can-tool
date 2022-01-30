#include <avrtos/kernel.h>

#include "app.h"

#include <stdio.h>

K_KERNEL_INIT();

int main()
{
        app_init();

        irq_enable();

        for (;;) {
                k_idle();
        }
}