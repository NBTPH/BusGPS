#ifndef _HTTP_WEBPAG_H
#define _HTTP_WEBPAG_H

#include <common.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_random.h"

void TaskWebpage(void *pvParameters);

#endif