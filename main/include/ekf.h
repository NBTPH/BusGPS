#ifndef _EKF_H_
#define _EKF_H_

#include <common.h>
#include <TinyEKF.h>
#include <i2c.h>

extern ekf_t KalmanFilter;
extern bool EKF_Origin_Set;

void ekf_init(float init_roll, float init_pitch, float init_yaw);
void ekf_set_origin(float init_Lat_origin, float init_Lon_origin);

void ekf_estimate(MPU6050_Sensor_t IMU, float timestep);
void ekf_update_tilt(MPU6050_Sensor_t IMU);
void ekf_update_heading(HMC5883_Sensor_t Mag);
void ekf_update_position(float Lat, float Lon, float SOG, float COG);

void get_tilt(float *roll, float *pitch);
void get_heading(float *heading);
void get_vel(float *vel_N, float *vel_E);
void get_pos(float *lat, float *lon);

#endif