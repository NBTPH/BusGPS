#ifndef _EKF_H_
#define _EKF_H_

#include <common.h>
#include <TinyEKF.h>
#include <i2c.h>

extern ekf_t KalmanFilter;

void ekf_init(float init_roll, float init_pitch, float init_yaw, float init_Lat_origin, float init_Lon_origin);
void ekf_estimate(MPU6050_Sensor_t IMU, float timestep);
void ekf_update_tilt(MPU6050_Sensor_t IMU);
void ekf_update_heading(HMC5883_Sensor_t Mag);
void ekf_update_position();

#endif