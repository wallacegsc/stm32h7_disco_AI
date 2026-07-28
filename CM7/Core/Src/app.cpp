// app.cpp
#include "main.h"
#include <cstdio>
#include "sys.h"
#include "usart.h"

#include "model.h"
#include "model_params.h"

#include "mpu6050.h"


#include <array>

class MotionSensor{
private:
    static constexpr int hz = 50; // Samples / second
    static constexpr int period = (int)1000/hz;
    static constexpr std::size_t BATCH_SIZE = hz;    

public:
    MotionSensor(
        MPU6050_Accelerometer_conf accel_config = MPU6050_Accelerometer_4G, 
        MPU6050_Gyroscope_conf gyro_config      = MPU6050_Gyroscope_1000_deg, 
        MPU6050_sample_rate_conf sample_rate    = MPU6050_DataRate_100Hz)
    :   acc_config{accel_config},
        gyr_config{gyro_config},
        rate_config{sample_rate}
    {};
    ~MotionSensor(){};

    struct MotionData{
        double Ax, Ay, Az;
        double Gx, Gy, Gz;
    };

    /*
    0->Sucess
    1->Fail
    */
    uint8_t init()
    {
        return MPU6050_Init(acc_config, gyr_config, rate_config);
    }

    MotionData get_sample()
    {
        MPU6050_t mpu_data;
        MPU6050_Read_All(&mpu_data);
        return mpu_2_mdata(mpu_data);
    }

    std::array<MotionData, BATCH_SIZE> get_batch_samples()
    {
        std::array<MotionData, BATCH_SIZE> batch;
        for (auto& sample : batch)
        {
            sample = get_sample();
            sys::delay(period);
        }
        return batch;
    }

private:

    uint8_t acc_config; 
    uint8_t gyr_config; 
    uint8_t rate_config;

    MotionData mpu_2_mdata(MPU6050_t& mpu_data)
    {
        MotionData motion_data;
        motion_data.Ax = mpu_data.Ax;
        motion_data.Ay = mpu_data.Ay;
        motion_data.Az = mpu_data.Az;
        
        motion_data.Gx = mpu_data.Gx;
        motion_data.Gy = mpu_data.Gy;
        motion_data.Gz = mpu_data.Gz;

        return motion_data;
    }
};


extern "C" void app_main(void)
{
    sys::UartSerial uart{huart1};
    sys::Logger log{uart};

    MotionSensor motion_sensor;
    
    if ( motion_sensor.init() != 0)
    {
        log.log_error("Motion Sensor Init Fail");
        return;
    }

    while(true)
    {
        log.log_info("-------------------");
        // 1 segundo bloqueado aqui, coletando 50 amostras
        auto batch = motion_sensor.get_batch_samples();
        for(auto& s: batch)
        {
            log.log_info("Acc-> X:%+4.3f Y:%+4.3f Z:%+4.3f   Giro-> X:%+8.3f Y:%+8.3f Z:%+8.3f",
                s.Ax, s.Ay, s.Az,
                s.Gx, s.Gy, s.Gz
            );
        }
    }

}