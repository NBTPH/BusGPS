#ifndef _WIFI_H
#define _WIFI_H

#include <common.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"

extern bool WiFi_Connected;
bool WiFi_init(bool ap_mode);
bool WiFi_start(bool ap_mode);

#endif