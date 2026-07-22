// app.cpp
#include "main.h"
#include <cstdio>
#include "sys.h"
#include "usart.h"

extern "C" void app_main(void)
{
    sys::UartSerial uart{huart1};
    sys::Logger log{uart};

    while (true) {
        log.log_info("Running");
        sys::delay(1000);
    }
}
