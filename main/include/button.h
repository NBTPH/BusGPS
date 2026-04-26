#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <common.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#define MPU6050_CALIB_BUTTON    GPIO_NUM_33
#define HMC5883_CALIB_BUTTON    GPIO_NUM_32

#define DEBOUNCE_INTERVAL_MS 10
#define DEBOUNCE_COUNT_LIMIT 3

extern QueueHandle_t Button_Queue;

void TaskButton(void *pvParameters);

#endif