#include "sys.h"

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <cstdarg> 

namespace sys {
    void delay(uint32_t ms){ HAL_Delay(ms);}


    int UartSerial::write_bytes(uint8_t* buffer, int len)
    {
        return HAL_UART_Transmit(&handle_, buffer, len, HAL_MAX_DELAY);
    }

    int UsbSerial::write_bytes(uint8_t* buffer, int len)
    {
        return -1;
    }

    void Logger::log_with_prefix(const char* prefix, const char* fmt, va_list args)
    {
        int offset = snprintf(buffer_, sizeof(buffer_), "%s", prefix);
        if (offset < 0) return;

        int n = vsnprintf(buffer_ + offset, sizeof(buffer_) - offset, fmt, args);
        if (n < 0) return;

        int total = offset + n;
        if (total > (int)sizeof(buffer_) - 1) total = sizeof(buffer_) - 1;  // clamp se truncou

        out_.write_bytes((uint8_t*)buffer_, total);
        out_.write_bytes((uint8_t*)"\r\n", 2);
    }

    void Logger::log_raw(const char* fmt, ...)
    {
        if(level_ >= Level::None) return;

        va_list args;
        va_start(args, fmt);

        int len = vsnprintf(buffer_, sizeof(buffer_), fmt, args);
        if (len > 0) {
            if (len > (int)sizeof(buffer_) - 1) len = sizeof(buffer_) - 1;  // truncou? ajusta
            out_.write_bytes((uint8_t*)buffer_, len);
        }

        va_end(args);
    }
    void Logger::log_info(const char* fmt, ...)
    {
        if(level_ > Level::Info) return;
        va_list args;
        va_start(args, fmt);
        this->log_with_prefix("I: ",fmt, args);
        va_end(args);
    }
    void Logger::log_error(const char* fmt, ...)
    {
        if(level_ > Level::Error) return;
        va_list args;
        va_start(args, fmt);
        this->log_with_prefix("E: ",fmt, args);
        va_end(args);
    }
    void Logger::log_debug(const char* fmt, ...)
    {
        if(level_ > Level::Debug) return;
        va_list args;
        va_start(args, fmt);
        this->log_with_prefix("D: ",fmt, args);
        va_end(args);
    }
}
