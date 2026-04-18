#ifndef GPS_H
#define GPS_H

#include <LC76G.h>
#include "driver/uart.h"

#define TX_PIN 17
#define RX_PIN 16

extern RMC_MSG_t GPS_data;

void TaskGPS(void *pvParameters);

#endif
