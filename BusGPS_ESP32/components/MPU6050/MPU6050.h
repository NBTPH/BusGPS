/*!
 *  @file Adafruit_MPU6050.h
 *
 * 	I2C Driver for MPU6050 6-DoF Accelerometer and Gyro
 *
 * 	This is a library for the Adafruit MPU6050 breakout:
 * 	https://www.adafruit.com/products/3886
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *
 *	BSD license (see license.txt)
 */

#ifndef _MPU6050_H
#define _MPU6050_H

#include "driver/i2c_master.h"

#define MPU6050_I2CADDR_DEFAULT 0x68 ///< MPU6050 default i2c address w/ AD0 low

typedef struct{
    float x;
    float y;
    float z;
}mpu6050_t;

extern bool MPU6050_is_Calibrating;

bool MPU6050_Init(i2c_master_dev_handle_t *input_mpu6050_dev);
void MPU6050_Calibration(uint32_t samples_num);

void getAccelRawData(int16_t *x, int16_t *y, int16_t *z);
void getGyroRawData(int16_t *x, int16_t *y, int16_t *z);

void getAccelData(float *x, float *y, float *z);
void getGyroData(float *x, float *y, float *z);

bool MPU6050_DataReady(void);

#endif