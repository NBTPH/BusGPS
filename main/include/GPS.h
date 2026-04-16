#ifndef GPS_H
#define GPS_H

#include <LC76G.h>
#include "driver/uart.h"

extern RMC_MSG_t GPS_data;

void TaskGPS(void *pvParameters);

#endif
