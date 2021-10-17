#include <avrtos/kernel.h>

#include "app.h"

int main()
{
        app_init();

        monitor_thread_entry(NULL);
}