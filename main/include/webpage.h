#ifndef _HTTP_WEBPAG_H
#define _HTTP_WEBPAG_H

#include <common.h>
#include <uart.h> //for data frame struct and gps fix status
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_http_server.h"
#include "esp_random.h"

typedef struct{
    uint32_t ID;
    Date_t Date;
    UTC_t Time;
    float Lat;
    float Lon;
    float Heading;
    bool Ignition;
    bool Door_Open;
    bool AC;
}DataFrame_t;

extern DataFrame_t Global_Data; //we are not using queue yet since the browser is the one asking for the data not the esp32 pushing it up

void TaskWebpage(void *pvParameters);

#endif