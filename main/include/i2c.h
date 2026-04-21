#ifndef I2C_H
#define I2C_H

#include <LSM303.h>
#include "driver/i2c_master.h"
#include <common.h>

#define SCL_PIN 22
#define SDA_PIN 21

void TaskI2C(void *pvParameters);

#endif
