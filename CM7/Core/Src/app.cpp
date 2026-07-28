// app.cpp
#include "main.h"
#include <cstdio>
#include "sys.h"
#include "usart.h"

#include "model.h"
#include "model_params.h"

#include "mpu6050.h"




extern "C" void app_main(void)
{
    sys::UartSerial uart{huart1};
    sys::Logger log{uart};

    MPU6050_Init(
        MPU6050_Accelerometer_4G, 
        MPU6050_Gyroscope_1000_deg, 
        MPU6050_DataRate_100Hz
    );

    MPU6050_t mpu_data;

    while(true)
    {
        MPU6050_Read_All(&mpu_data);

        log.log_info("Acc-> X:%+4.3f Y:%+4.3f Z:%+4.3f   Giro-> X:%+8.3f Y:%+8.3f Z:%+8.3f Ang-> X:%+8.3f Y:%+8.3f",
            mpu_data.Ax, mpu_data.Ay, mpu_data.Az,
            mpu_data.Gx, mpu_data.Gy, mpu_data.Gz,
            mpu_data.KalmanAngleX, mpu_data.KalmanAngleY 
        );

        // log.log_info("Running...");
        sys::delay(50);
    }

}