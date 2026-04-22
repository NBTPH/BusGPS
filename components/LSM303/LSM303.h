#ifndef __LSM303_H__
#define __LSM303_H__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "driver/i2c_master.h"
#include "esp_check.h"

#define SENSORS_GRAVITY_EARTH (9.80665F)
#define SENSORS_GRAVITY_STANDARD (SENSORS_GRAVITY_EARTH)

#define LSM303_ADDRESS_ACCEL          (0x32 >> 1)         // 0011001x
#define LSM303_ADDRESS_MAG            (0x3C >> 1)         // 0011110x

#define LSM303_REGISTER_ACCEL_WHO_AM_I              0x0F
#define LSM303_REGISTER_ACCEL_CTRL_REG1_A           0x20   // 00000111   rw
#define LSM303_REGISTER_ACCEL_CTRL_REG2_A           0x21   // 00000000   rw
#define LSM303_REGISTER_ACCEL_CTRL_REG3_A           0x22   // 00000000   rw
#define LSM303_REGISTER_ACCEL_CTRL_REG4_A           0x23   // 00000000   rw
#define LSM303_REGISTER_ACCEL_CTRL_REG5_A           0x24   // 00000000   rw
#define LSM303_REGISTER_ACCEL_CTRL_REG6_A           0x25   // 00000000   rw
#define LSM303_REGISTER_ACCEL_REFERENCE_A           0x26   // 00000000   r
#define LSM303_REGISTER_ACCEL_STATUS_REG_A          0x27   // 00000000   r
#define LSM303_REGISTER_ACCEL_OUT_X_L_A             0x28
#define LSM303_REGISTER_ACCEL_OUT_X_H_A             0x29
#define LSM303_REGISTER_ACCEL_OUT_Y_L_A             0x2A
#define LSM303_REGISTER_ACCEL_OUT_Y_H_A             0x2B
#define LSM303_REGISTER_ACCEL_OUT_Z_L_A             0x2C
#define LSM303_REGISTER_ACCEL_OUT_Z_H_A             0x2D
#define LSM303_REGISTER_ACCEL_FIFO_CTRL_REG_A       0x2E
#define LSM303_REGISTER_ACCEL_FIFO_SRC_REG_A        0x2F
#define LSM303_REGISTER_ACCEL_INT1_CFG_A            0x30
#define LSM303_REGISTER_ACCEL_INT1_SOURCE_A         0x31
#define LSM303_REGISTER_ACCEL_INT1_THS_A            0x32
#define LSM303_REGISTER_ACCEL_INT1_DURATION_A       0x33
#define LSM303_REGISTER_ACCEL_INT2_CFG_A            0x34
#define LSM303_REGISTER_ACCEL_INT2_SOURCE_A         0x35
#define LSM303_REGISTER_ACCEL_INT2_THS_A            0x36
#define LSM303_REGISTER_ACCEL_INT2_DURATION_A       0x37
#define LSM303_REGISTER_ACCEL_CLICK_CFG_A           0x38
#define LSM303_REGISTER_ACCEL_CLICK_SRC_A           0x39
#define LSM303_REGISTER_ACCEL_CLICK_THS_A           0x3A
#define LSM303_REGISTER_ACCEL_TIME_LIMIT_A          0x3B
#define LSM303_REGISTER_ACCEL_TIME_LATENCY_A        0x3C
#define LSM303_REGISTER_ACCEL_TIME_WINDOW_A         0x3D

#define LSM303_REGISTER_MAG_CRA_REG_M               0x00
#define LSM303_REGISTER_MAG_CRB_REG_M               0x01
#define LSM303_REGISTER_MAG_MR_REG_M                0x02
#define LSM303_REGISTER_MAG_OUT_X_H_M               0x03
#define LSM303_REGISTER_MAG_OUT_X_L_M               0x04
#define LSM303_REGISTER_MAG_OUT_Z_H_M               0x05
#define LSM303_REGISTER_MAG_OUT_Z_L_M               0x06
#define LSM303_REGISTER_MAG_OUT_Y_H_M               0x07
#define LSM303_REGISTER_MAG_OUT_Y_L_M               0x08
#define LSM303_REGISTER_MAG_SR_REG_Mg               0x09
#define LSM303_REGISTER_MAG_IRA_REG_M               0x0A
#define LSM303_REGISTER_MAG_IRB_REG_M               0x0B
#define LSM303_REGISTER_MAG_IRC_REG_M               0x0C
#define LSM303_REGISTER_MAG_TEMP_OUT_H_M            0x31
#define LSM303_REGISTER_MAG_TEMP_OUT_L_M            0x32

//MAGNETOMETER GAIN SETTINGS
typedef enum{
    LSM303_MAGGAIN_1_3                        = 0x20,  // +/- 1.3
    LSM303_MAGGAIN_1_9                        = 0x40,  // +/- 1.9
    LSM303_MAGGAIN_2_5                        = 0x60,  // +/- 2.5
    LSM303_MAGGAIN_4_0                        = 0x80,  // +/- 4.0
    LSM303_MAGGAIN_4_7                        = 0xA0,  // +/- 4.7
    LSM303_MAGGAIN_5_6                        = 0xC0,  // +/- 5.6
    LSM303_MAGGAIN_8_1                        = 0xE0   // +/- 8.1
}lsm303MagGain;

//MAGNETOMETER UPDATE RATE SETTINGS
typedef enum{
    LSM303_MAGRATE_0_7                        = 0x00,  // 0.75 Hz
    LSM303_MAGRATE_1_5                        = 0x01,  // 1.5 Hz
    LSM303_MAGRATE_3_0                        = 0x02,  // 3.0 Hz
    LSM303_MAGRATE_7_5                        = 0x03,  // 7.5 Hz
    LSM303_MAGRATE_15                         = 0x04,  // 15 Hz
    LSM303_MAGRATE_30                         = 0x05,  // 30 Hz
    LSM303_MAGRATE_75                         = 0x06,  // 75 Hz
    LSM303_MAGRATE_220                        = 0x07   // 200 Hz
}lsm303MagRate;

//Set of linear acceleration measurement ranges
typedef enum range{
    LSM303_RANGE_2G = 0b00,  ///< Measurement range from +2G to -2G (19.61 m/s^2)
    LSM303_RANGE_4G = 0b01,  ///< Measurement range from +4G to -4G (39.22 m/s^2)
    LSM303_RANGE_8G = 0b10,  ///< Measurement range from +8G to -8G (78.45 m/s^2)
    LSM303_RANGE_16G = 0b11, ///< Measurement range from +16G to -16G (156.9 m/s^2)
}lsm303_accel_range_t;

//Set of different modes that can be used. Normal, high resolution, and low power
typedef enum mode {
    LSM303_MODE_NORMAL,          ///< Normal measurement mode; 10-bit
    LSM303_MODE_HIGH_RESOLUTION, ///< High resolution mode; 12-bit
    LSM303_MODE_LOW_POWER,       ///< Low power mode; 8-bit
}lsm303_accel_mode_t;

//Set of different data rates that can be used.
typedef enum datarate {
    LSM303_DATARATE_1 = 1,
    LSM303_DATARATE_10 = 2,
    LSM303_DATARATE_25 = 3,
    LSM303_DATARATE_50 = 4,
    LSM303_DATARATE_100 = 5,
    LSM303_DATARATE_200 = 6,
    LSM303_DATARATE_400 = 7,
    LSM303_DATARATE_LOWPOWER_1620 = 8,
    LSM303_DATARATE_1344_LOWPOWER_5376 = 9,
}lsm303_accel_datarate_t;

//INTERNAL MAGNETOMETER DATA TYPE
typedef struct lsm303MagData_s{
    int16_t x;
    int16_t y;
    int16_t z;
}lsm303MagData;

//INTERNAL ACCELERATION DATA TYPE
typedef struct lsm303AccelData_s{
    int16_t x;
    int16_t y;
    int16_t z;
}lsm303AccelData;

#define LSM303_ID                     (0b11010100)

bool LSM303_init(i2c_master_dev_handle_t *input_i2c_MagDev, i2c_master_dev_handle_t *input_i2c_AccelDev);

bool get_Accel_Data(float *x, float *y, float *z);

bool get_Mag_Data(float *x, float *y, float *z);

#endif