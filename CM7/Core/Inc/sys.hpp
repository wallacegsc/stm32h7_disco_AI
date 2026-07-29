#pragma once 

#include <cstdint>
#include <array>
#include "serial.hpp"
#include "stm32h7xx_hal.h"

namespace sys {
    void delay(uint32_t ms);
    uint32_t get_miliseconds();
}

class UartSerial : public Serial{
    UART_HandleTypeDef& handle_;
    
    constexpr static std::size_t RX_SIZE = 10;
    std::array<uint8_t, RX_SIZE> rx_ring_{};
    volatile std::size_t head_ = 0;   // written by ISR
    volatile std::size_t tail_ = 0;   // Readed
    uint8_t rx_byte_ = 0;

public:
    explicit UartSerial(UART_HandleTypeDef& handle) : handle_{handle} {};

    void start_rx()
    {
        HAL_UART_Receive_IT(&handle_, &rx_byte_, 1); 
    }

    void on_rx_isr()
    {
        std::size_t next = (head_ + 1) % RX_SIZE;
        if(next != tail_)
        {
            rx_ring_[head_] = rx_byte_;
            head_ = next;
        }
        start_rx();
    }

    int read_bytes(uint8_t* buffer, int len) override
    {
        int n = 0;
        while(n < len && head_ != tail_)
        {
            buffer[n++] = rx_ring_[tail_];
            tail_ = (tail_ + 1 ) % RX_SIZE;
        }
        return n;
    }

    int write_bytes(const uint8_t* buffer, int len) override
    {
        return HAL_UART_Transmit(&handle_, buffer, len, HAL_MAX_DELAY);
    }


    bool available() const override
    {
        return head_ != tail_;
    } 

};