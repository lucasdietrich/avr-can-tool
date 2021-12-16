#include <avrtos/kernel.h>

#include "app.h"

#include <stdio.h>

int main()
{
        app_init();
        
        for (;;) {
                k_sleep(K_SECONDS(5));
        }
}