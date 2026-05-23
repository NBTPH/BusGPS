#ifndef _UART_H
#define _UART_H

#include <common.h>
#include <NMEA.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"

#define TX_PIN 17
#define RX_PIN 16

#define GPS_QUEUE_LENGTH 10

typedef struct{
    Date_t DATE;
    UTC_t UTC;
    float Lat;
    float Lon;
    float Alt;
    float SOG; //Knots
    float COG; //Degree
}GPS_t;

extern QueueHandle_t GPS_Queue;
extern bool GPS_Fixed;
void TaskUART(void *pvParameters);

#endif
