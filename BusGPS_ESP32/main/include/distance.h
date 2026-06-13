#ifndef _DISTANCE_H
#define _DISTANCE_H

#include <common.h>
#include <i2c.h> //for calibration flags
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#define TRIG_PIN GPIO_NUM_27
#define ECHO_PIN GPIO_NUM_26

#define DISTANCE_QUEUE_LENGTH 10

#define DISTANCE_DOOR_OPEN_THRESHOLD 0.2f

extern QueueHandle_t Distance_Queue;

void TaskDoor(void *pvParameters);

#endif