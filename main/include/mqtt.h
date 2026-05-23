#ifndef _MQTT_H
#define _MQTT_H

#include <common.h> //this MQTT part mostly use the same networking stuff from webpage
#include <uart.h> //for data frame struct to send out and gps fix status
#include <wifi.h> //for wifi connected state
#include <i2c.h> //for calibration flags
#include "mqtt_client.h"

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

extern DataFrame_t Global_Data; //we are not using queue browser is the one asking for the data not the esp32 pushing it up and mqtt will have a flag of its own to send out data
bool DataFrame_2_JSON(char *buffer, size_t max_length, DataFrame_t data);

void MQTT_Init();
void MQTT_Send_MSG(char *buff, size_t length);
void TaskMQTT(void *pvParameters);

#endif