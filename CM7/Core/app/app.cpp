// app.cpp
#include <cstdio>

#include "sys.hpp"
#include "serial.hpp"
#include "motion_sensor.hpp"

#include "usart.h"

UartSerial uart{huart1};
LoggerSerial logger{uart};
MenuSerial menu{uart};
MotionSensor motion_sensor;

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        uart.on_rx_isr();
    }
}

using OptionState = MenuSerial::OptionState;
using MenuState   = MenuSerial::MenuState;

OptionState motion_init(std::string_view args, MenuState menu_state)
{
    if (motion_sensor.force_reinit() != 0){
        logger.log_error("Motion Sensor Init Fail");
        return OptionState::Finish;
    }
    logger.log_info("Success [if futher readings fail try motion_init again]");

    return OptionState::Finish;
}

OptionState motion_get_batch(std::string_view args, MenuState menu_state)
{
    if (motion_sensor.init() != 0){
        logger.log_error("Motion Sensor Init Fail");
        return OptionState::Finish;
    }

    for (auto& s : motion_sensor.get_batch_samples())
        logger.log_info("Acc-> X:%+4.3f Y:%+4.3f Z:%+4.3f   Giro-> X:%+8.3f Y:%+8.3f Z:%+8.3f",
                        s.Ax, s.Ay, s.Az, s.Gx, s.Gy, s.Gz);

    return OptionState::Finish;
}

OptionState motion_get_samples(std::string_view args, MenuState menu_state)
{
    if(menu_state == MenuState::Stop)
        return OptionState::Finish;
    //loop
    if (motion_sensor.init() != 0){
        logger.log_error("Motion Sensor Init Fail");
        return OptionState::Finish;
    }

    MotionSensor::MotionData s = motion_sensor.get_sample();
    logger.log_info("Acc-> X:%+4.3f Y:%+4.3f Z:%+4.3f   Giro-> X:%+8.3f Y:%+8.3f Z:%+8.3f",
                    s.Ax, s.Ay, s.Az, s.Gx, s.Gy, s.Gz);
    
    sys::delay(200);
            
    return OptionState::Running;
}

MenuSerial::option ops[] = 
{
    {
        .cmd  = "motion_init",
        .help = "Initializes mpu6050",
        .f    = &motion_init
    },
    {
        .cmd  = "motion_get_batch",
        .help = "Measures batch samples",
        .f    = &motion_get_batch
    },
    {
        .cmd  = "motion_get_samples",
        .help = "Get samples in a loop",
        .f    = &motion_get_samples
    },
};

extern "C" void app_main(void)
{

    for(MenuSerial::option op : ops)
        menu.instance_option(op);

    uart.start_rx();
    while(1)
    {
        menu.run();
    }
}