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

#ifndef _HMC5883_H
#define _HMC5883_H

#define HMC5883_ADDRESS_MAG (0x3C >> 1) // 0011110x

extern bool HMC5883_is_Calibrating;

void HMC5883_Calibration(uint32_t samples_num);
bool HMC5883_Init(i2c_master_dev_handle_t *input_hmc5883_dev);

void getMagRawData(int16_t *x, int16_t *y, int16_t *z);
void getMagData(float *x, float *y, float *z);

bool HMC5883_DataReady(void);

#endif