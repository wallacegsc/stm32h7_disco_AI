#ifndef SYS_H
#define SYS_H

#include <cstdint>
#include "stm32h7xx_hal.h"
#include <cstdarg>
namespace sys {
    void delay(uint32_t ms);

    class Serial{
    public:
        virtual int write_bytes(uint8_t* buffer, int len) = 0;
        virtual ~Serial() = default;
    private:
    };  

    class UartSerial : public Serial{
        UART_HandleTypeDef& handle_;
    public:
        explicit UartSerial(UART_HandleTypeDef& handle) : handle_{handle} {};

        int write_bytes(uint8_t* buffer, int len) override;

    };

    class UsbSerial: public Serial{
    public:
        UsbSerial(){}

        int write_bytes(uint8_t* buffer, int len) override;
    };

    class Logger{
        Serial& out_;
        static constexpr size_t BUF_SIZE = 128;
        char buffer_[BUF_SIZE];

    public:
        enum class Level {Debug = 0, Info = 1, Error = 2, None = 3};

        explicit Logger(Serial& s) : out_{s}{}

        void set_level(Level l) { level_ = l; }

        void log_raw(const char* fmt, ...);
        void log_info(const char* fmt, ...);
        void log_error(const char* fmt, ...);
        void log_debug(const char* fmt, ...);

    private:
        Level level_ = Level::Debug;
        
        void log_with_prefix(const char* prefix, const char* fmt, va_list args);
    };
}

#endif /*SYS_H*/
