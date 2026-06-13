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
#include <distance.h>
#include <storage.h>
#include <ekf.h>
#include <webpage.h>
#include <mqtt.h>
#include <wifi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


void Calibration_TaskHalt(TaskHandle_t *TaskHalt_arr){
    printf("Calibration Halting Tasks\n");
    for(uint8_t i = 0; i < 4; i++){
        vTaskSuspend(TaskHalt_arr[i]);
    }
}

void Calibration_TaskResume(TaskHandle_t *TaskHalt_arr){
    printf("Calibration Resuming Tasks\n");
    for(uint8_t i = 0; i < 4; i++){
        vTaskResume(TaskHalt_arr[i]);
    }
}

void app_main(void){
    gpio_config_t config = {0};
    config.mode = GPIO_MODE_OUTPUT;
    config.pin_bit_mask = (1ULL << GPIO_NUM_2);
    ESP_ERROR_CHECK(gpio_config(&config));

    MPU6050_Sensor_t MPU6050_Data = {0};
    int64_t MPU6050_Data_Interval = 0;
    HMC5883_Sensor_t HMC5883_Data = {0};
    DHT20_Sensor_t DHT20_Data = {0};
    GPS_t GPS_Data = {0};
    float Distance_Data = 0;
    Global_Data.ID = 111205;

    TaskHandle_t CalibTaskArr[4] = {0};
    TaskHandle_t TaskMQTT_handle = NULL;

    flash_storage_init();

    WiFi_init(false); //init wifi periferal
    if(WiFi_start(false)){//only when wifi starts and connected succesfully do we run HTTP server and MQTT tasks
        xTaskCreate(TaskWebpage, "Webpage HTTP Webserver hosting", 4096, NULL, 1, NULL);
        xTaskCreate(TaskMQTT, "MQTT send messages", 4096, NULL, 2, &TaskMQTT_handle);
    }

    xTaskCreate(TaskGY87, "GY87 IMU/Mag read task", 4096, NULL, 3, NULL);
    xTaskCreate(TaskDHT20, "DHT20 read task", 4096, NULL, 2, &CalibTaskArr[0]);
    xTaskCreate(TaskUART, "UART/GPS read", 4096, NULL, 3, &CalibTaskArr[1]);
    xTaskCreate(TaskDoor, "Door State/Ultra Sonic sensor read", 2048, NULL, 2, &CalibTaskArr[2]);
    xTaskCreate(TaskButton, "Button debounce task", 2048, NULL, 1, &CalibTaskArr[3]);
    
    bool blink_on = true;
    int64_t last_print = millis();
    int64_t last_blink = millis();
    int64_t last_WDT_feed = millis();
    uint8_t MPU6050_Polling_Count = 0;

    delay(1); //wait for queues to be established in other tasks before we call receive
    vTaskPrioritySet(NULL, 3);  //change main task to have higher priority than button task so that main task takes in and process data faster

    ekf_init(0.01, 0.01, 0.5);
    while(1){
        int64_t current_millis = millis();
        uint8_t msg_button = 5;

        //Receiving data from MPU6050 (accelerometer & gyroscope)
        MPU6050_Sensor_t MPU6050_Data_Temp = {0};
        if(xQueueReceive(MPU6050_Queue, (void *)&MPU6050_Data_Temp, 1) == pdTRUE){
            MPU6050_Data_Interval = MPU6050_Data_Temp.Timestamp - MPU6050_Data.Timestamp;
            MPU6050_Data = MPU6050_Data_Temp;
            ekf_estimate(MPU6050_Data, (MPU6050_Data_Interval / 1000000.0f));
            
            MPU6050_Polling_Count++;
            if(MPU6050_Polling_Count >= 4){
                ekf_update_tilt(MPU6050_Data);
                MPU6050_Polling_Count = 0;
            }
        }

        //Receiving data from HMC5883 (magnetometer)
        if(xQueueReceive(HMC5883_Queue, (void *)&HMC5883_Data, 1) == pdTRUE){
            float headingRad = atan2(-HMC5883_Data.y, HMC5883_Data.x); //calculate heading, the output is the angle value in radiant of North relative to X axis
            debug_Heading = (headingRad * 180) / M_PI;
            float declinationAngle = -0.64166666666667; //declination angle in HCMC as of April 2026
            debug_Heading += declinationAngle;
            if (debug_Heading < 0) debug_Heading += 360;

            ekf_update_heading(HMC5883_Data);
            get_heading(&Global_Data.Heading);
        }

        //Receiving data from DHT20 (temperature & humidity)
        if(xQueueReceive(DHT20_Queue, (void *)&DHT20_Data, 0) == pdTRUE){ //no timeout and returns immediately because this sensor takes longer time
            if(isnan(DHT20_Data.temp)){
                Global_Data.Ignition = 0; 
            }
            else if(DHT20_Data.temp < ENGINE_IGNITION_THRESHOLD){
                Global_Data.Ignition = 1; 
            }
            else{
                Global_Data.Ignition = 2; 
            }
        }

        //Receiving data from HC-SR04 (ultrasonic distance sensor)
        if(xQueueReceive(Distance_Queue, (void *)&Distance_Data, 0) == pdTRUE){ //no timeout and returns immediately because this sensor takes longer time
            if(isnan(Distance_Data)){
                Global_Data.Door_Open = 0;
            }
            else if(Distance_Data < DISTANCE_DOOR_OPEN_THRESHOLD){
                Global_Data.Door_Open = 1; 
            }
            else{
                Global_Data.Door_Open = 2; 
            }
        }

        //Receiving data from GPS (ultrasonic distance sensor)
        if(GPS_Fixed){ //only taking in queue data when GPS is fixed
            if(xQueueReceive(GPS_Queue, (void *)&GPS_Data, 1) == pdTRUE && (!MPU6050_is_Calibrating && !HMC5883_is_Calibrating)){ //only processing GPS when there are data arrives and sensors are not calibrating
                if(!EKF_Origin_Set){ //if the ekf origin has not been set
                    ekf_set_origin(GPS_Data.Lat, GPS_Data.Lon, GPS_Data.SOG, GPS_Data.COG); //set the origin
                }
                ekf_update_position(GPS_Data.Lat, GPS_Data.Lon, GPS_Data.SOG, GPS_Data.COG);
                debug_Lat = GPS_Data.Lat;
                debug_Lon = GPS_Data.Lon;
                Global_Data.Date = GPS_Data.DATE;
                Global_Data.Time = GPS_Data.UTC;
                get_pos(&Global_Data.Lat, &Global_Data.Lon);

                xTaskNotifyGive(TaskMQTT_handle); //notify MQTT task to send message
            }
        }

        //Detect button press action
        if(xQueueReceive(Button_Queue, (void *)&msg_button, 0) == pdTRUE){
            switch (msg_button){
                case 0:
                    gpio_intr_disable(ECHO_PIN);
                    Calibration_TaskHalt(CalibTaskArr);
                    MPU6050_Calibration(5000);
                    Calibration_TaskResume(CalibTaskArr);
                    break;
                case 1:
                    gpio_intr_disable(ECHO_PIN);
                    Calibration_TaskHalt(CalibTaskArr);
                    HMC5883_Calibration(200);
                    Calibration_TaskResume(CalibTaskArr);
                    break;
                default:
                    break;
            }
        }

        if(current_millis - last_print >= 1000 && true){ //print debug
            last_print = current_millis;
            // printf("ACCEL X: %5f Y: %5f Z: %5f\r\n", MPU6050_Data.Accel.x, MPU6050_Data.Accel.y, MPU6050_Data.Accel.z);
            // printf("GYRO X: %5f Y: %5f Z: %5f\r\n", MPU6050_Data.Gyro.x, MPU6050_Data.Gyro.y, MPU6050_Data.Gyro.z);
            printf("INTERVAL: %lld us\r\n", MPU6050_Data_Interval);
            printf("KALMAN FILTER: ROLL %5f PITCH %5f\r\n", (KalmanFilter.x[0] * (180.0f / M_PI)), (KalmanFilter.x[1] * (180.0f / M_PI)));
            // printf("MAG X: %5f Y: %5f Z: %5f\r\n", HMC5883_Data.x, HMC5883_Data.y, HMC5883_Data.z);
            printf("HEADING: %10.6f degree\r\n", debug_Heading);
            printf("DISTANCE: %10.6f meters\r\n", Distance_Data);
            printf("TEMPERATURE: %10.6f degree\r\n", DHT20_Data.temp);
            printf("KALMAN FILTER HEADING: %10.6f degree\r\n\n\n", (KalmanFilter.x[2] * (180.0f / M_PI)));
        }

        if(current_millis - last_blink >= 1000){ //simple blink LED
            last_blink = current_millis;
            gpio_set_level(GPIO_NUM_2, blink_on);
            blink_on = !blink_on;
        }

        if(current_millis - last_WDT_feed >= 4.8 * 1000){ //watchdog feed
            last_WDT_feed = current_millis;
            delay(1);
        }
    }
}