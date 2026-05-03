/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <common.h>
#include <math.h>
#include <uart.h>
#include <i2c.h>
#include <button.h>
#include <storage.h>
#include <ekf.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void app_main(void){
    gpio_config_t config = {0};
    config.mode = GPIO_MODE_OUTPUT;
    config.pin_bit_mask = (1ULL << GPIO_NUM_2);
    ESP_ERROR_CHECK(gpio_config(&config));

    MPU6050_Sensor_t MPU6050_Data = {0};
    int64_t MPU6050_Data_Interval = 0;
    HMC5883_Sensor_t HMC5883_Data = {0};
    float Heading = 0;

    flash_storage_init();

    xTaskCreate(TaskUART, "TaskUART/GPS read", 4096, NULL, 2, NULL);
    xTaskCreate(TaskI2C, "IMU/Mag read task", 4096, NULL, 3, NULL); //4KB stack
    xTaskCreate(TaskButton, "Button debounce task", 2048, NULL, 1, NULL); //2KB stack

    bool blink_on = true;
    int64_t last_print = millis();
    int64_t last_blink = millis();
    int64_t last_WDT_feed = millis();

    delay(1); //wait for queues to be established in other tasks
    vTaskPrioritySet(NULL, 2);  //change main task to have higher priority than button task so that main task takes in data faster 

    ekf_init(0.01, 0.01, 0.5, 0.01, 0.01);
    while(1){
        int64_t current_millis = millis();
        uint8_t msg_button = 5;

        MPU6050_Sensor_t MPU6050_Data_Temp = {0};
        if(xQueueReceive(MPU6050_Queue, (void *)&MPU6050_Data_Temp, 1) == pdTRUE){
            MPU6050_Data_Interval = MPU6050_Data_Temp.Timestamp - MPU6050_Data.Timestamp;
            MPU6050_Data = MPU6050_Data_Temp;
            ekf_estimate(MPU6050_Data, (MPU6050_Data_Interval / 1000000.0f));
            ekf_update_tilt(MPU6050_Data);
        }
        if(xQueueReceive(HMC5883_Queue, (void *)&HMC5883_Data, 1) == pdTRUE){
            float headingRad = atan2(-HMC5883_Data.y, HMC5883_Data.x); //calculate heading, the output is the angle value in radiant of North relative to X axis
            Heading = (headingRad * 180) / M_PI;
            float declinationAngle = -0.64166666666667; //declination angle in HCMC as of April 2026

            Heading += declinationAngle;

            if (Heading < 0)    Heading += 360;
            ekf_update_heading(HMC5883_Data);
        }
        if(xQueueReceive(Button_Queue, (void *)&msg_button, 0) == pdTRUE){
            switch (msg_button){
                case 0:
                    MPU6050_Calibration(5000);
                    break;
                case 1:
                    HMC5883_Calibration(200);
                    break;
                default:
                    break;
            }
        }

        if(current_millis - last_print >= 100){
            last_print = current_millis;
            // printf("ACCEL X: %5f Y: %5f Z: %5f\r\n", MPU6050_Data.Accel.x, MPU6050_Data.Accel.y, MPU6050_Data.Accel.z);
            // printf("GYRO X: %5f Y: %5f Z: %5f\r\n", MPU6050_Data.Gyro.x, MPU6050_Data.Gyro.y, MPU6050_Data.Gyro.z);
            // printf("INTERVAL: %lld us\r\n", MPU6050_Data_Interval);
            printf("KALMAN FILTER: ROLL %5f PITCH %5f\r\n\n", (KalmanFilter.x[0] * (180.0f / M_PI)), (KalmanFilter.x[1] * (180.0f / M_PI)));
            // printf("MAG X: %5f Y: %5f Z: %5f\r\n", HMC5883_Data.x, HMC5883_Data.y, HMC5883_Data.z);
            printf("HEADING: %10.6f degree\r\n", Heading);
            printf("KALMAN FILTER HEADING: %10.6f degree\r\n\n\n", (KalmanFilter.x[2] * (180.0f / M_PI)));
        }

        if(current_millis - last_blink >= 1000){
            last_blink = current_millis;
            gpio_set_level(GPIO_NUM_2, blink_on);
            blink_on = !blink_on;
        }

        if(current_millis - last_WDT_feed >= 4.8 * 1000){
            last_WDT_feed = current_millis;
            vTaskDelay(1);
        }
    }
}