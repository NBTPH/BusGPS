#ifndef _I2C_H
#define _I2C_H

#include <common.h>
#include <MPU6050.h>
#include <HMC5883.h>
#include <DHT20.h>
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"

#define SCL_PIN 22
#define SDA_PIN 21

#define SCL_PIN2 19
#define SDA_PIN2 18

#define MPU6050_QUEUE_LENGTH 10
#define HMC5883_QUEUE_LENGTH 10
#define DHT20_QUEUE_LENGTH 10

#define ENGINE_IGNITION_THRESHOLD 40

typedef struct{
    mpu6050_t Accel;
    mpu6050_t Gyro;
    int64_t Timestamp;
}MPU6050_Sensor_t;

typedef struct{
    float x;
    float y;
    float z;
}HMC5883_Sensor_t;

typedef struct{
    float humi;
    float temp;
}DHT20_Sensor_t;

extern QueueHandle_t MPU6050_Queue;
extern QueueHandle_t HMC5883_Queue;
extern QueueHandle_t DHT20_Queue;

void TaskGY87(void *pvParameters);
void TaskDHT20(void *pvParameters);

#endif
