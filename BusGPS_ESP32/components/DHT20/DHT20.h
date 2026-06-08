#ifndef _DHT20_H
#define _DHT20_H

#include <common.h>
#include "esp_check.h"
#include "driver/i2c_master.h"

#define DHT20_I2C_ADDRESS 0x38 // 0111000x

bool DHT20_Init(i2c_master_dev_handle_t *input_dht20_dev);
bool DHT20_ReadData(void);
int DHT20_ReadData_NonBlocking(void);
void DHT20_GetData(float *ptr_humi, float *ptr_temp);
float DHT20_GetHumidity(void);
float DHT20_GetTemperature(void);

#endif