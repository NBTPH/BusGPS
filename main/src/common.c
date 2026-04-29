#include <common.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"

int64_t millis(){
    int64_t result = esp_timer_get_time() / 1000;
    return result;
}

int64_t micros(){
    return esp_timer_get_time();
}

void delay(uint32_t ms){
    vTaskDelay(pdMS_TO_TICKS(ms));
}

int debug_printf(const char *fmt, ...){
    va_list args;
    int result;

    va_start(args, fmt);

#if DEBUG_PRINT == 1
    // Forward the variadic arguments to vprintf
    result = vprintf(fmt, args);
#else
    // Determine the number of characters that would be printed 
    // without actually outputting anything.
    result = vsnprintf(NULL, 0, fmt, args);
#endif

    va_end(args);
    return result;
}