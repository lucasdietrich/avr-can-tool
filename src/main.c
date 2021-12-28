#include <avrtos/kernel.h>

#include "app.h"

#include <stdio.h>

int main()
{
        app_init();

        irq_enable();

        for (;;) {
                k_idle();
        }
}