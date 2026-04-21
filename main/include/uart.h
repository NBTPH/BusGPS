#ifndef GPS_H
#define GPS_H

#include <LC76G.h>
#include "driver/uart.h"
#include <common.h>

#define TX_PIN 17
#define RX_PIN 16

extern RMC_MSG_t GPS_RMC_data;
extern GGA_MSG_t GPS_GGA_data;

void TaskGPS(void *pvParameters);

#endif
