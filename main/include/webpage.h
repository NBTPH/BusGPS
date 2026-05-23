#ifndef _HTTP_WEBPAG_H
#define _HTTP_WEBPAG_H

#include <common.h>
#include "esp_http_server.h"

extern float debug_Heading;
extern float debug_Lat;
extern float debug_Lon;

void TaskWebpage(void *pvParameters);

#endif