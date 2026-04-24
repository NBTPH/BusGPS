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

bool HMC5883_Init(i2c_master_dev_handle_t *input_hmc5883_dev);

void getMagData(float *x, float *y, float *z);

bool HMC5883_DataReady(void);

// mpu6050_accel_range_t getAccelerometerRange(void);
// void setAccelerometerRange(mpu6050_accel_range_t);

// mpu6050_gyro_range_t getGyroRange(void);
// void setGyroRange(mpu6050_gyro_range_t);

// void setInterruptPinPolarity(bool active_low);
// void setInterruptPinLatch(bool held);
// void setFsyncSampleOutput(mpu6050_fsync_out_t fsync_output);

// mpu6050_highpass_t getHighPassFilter(void);
// void setHighPassFilter(mpu6050_highpass_t bandwidth);

// void setMotionInterrupt(bool active);
// void setMotionDetectionThreshold(uint8_t thr);
// void setMotionDetectionDuration(uint8_t dur);
// bool getMotionInterruptStatus(void);

// mpu6050_fsync_out_t getFsyncSampleOutput(void);
// void setI2CBypass(bool bypass);

// void setClock(mpu6050_clock_select_t);
// mpu6050_clock_select_t getClock(void);

// void setFilterBandwidth(mpu6050_bandwidth_t bandwidth);
// mpu6050_bandwidth_t getFilterBandwidth(void);

// void setSampleRateDivisor(uint8_t);
// uint8_t getSampleRateDivisor(void);

// bool enableSleep(bool enable);
// bool enableCycle(bool enable);

// void setCycleRate(mpu6050_cycle_rate_t rate);
// mpu6050_cycle_rate_t getCycleRate(void);

// bool setGyroStandby(bool xAxisStandby, bool yAxisStandby, bool zAxisStandby);
// bool setAccelerometerStandby(bool xAxisStandby, bool yAxisStandby, bool zAxisStandby);
// bool setTemperatureStandby(bool enable);

// void reset(void);

// void getTemperatureSensor(void);
// void getAccelerometerSensor(void);
// void getGyroSensor(void);


// void _getRawSensorData(void);
// void _scaleSensorData(void);

#endif