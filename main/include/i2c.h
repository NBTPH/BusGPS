#ifndef _I2C_H
#define _I2C_H

#include <common.h>
#include <MPU6050.h>
#include <HMC5883.h>
#include "driver/i2c_master.h"

#define SCL_PIN 22
#define SDA_PIN 21

void TaskI2C(void *pvParameters);

#endif
