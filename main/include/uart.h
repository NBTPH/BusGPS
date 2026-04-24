#ifndef _UART_H
#define _UART_H

#include <common.h>
#include <NMEA.h>
#include "driver/uart.h"

#define TX_PIN 17
#define RX_PIN 16

extern RMC_MSG_t GPS_RMC_data;
extern GGA_MSG_t GPS_GGA_data;

void TaskUART(void *pvParameters);

#endif
