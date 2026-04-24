#include <common.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

int64_t millis(){
    int64_t result = esp_timer_get_time() / 1000;
    return result;
}

void delay(uint32_t ms){
    vTaskDelay(pdMS_TO_TICKS(ms));
}