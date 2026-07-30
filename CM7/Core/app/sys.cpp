#include "sys.hpp"

namespace sys {
    void delay(uint32_t ms) { HAL_Delay(ms); }
    uint32_t get_miliseconds() { return HAL_GetTick(); }
}


