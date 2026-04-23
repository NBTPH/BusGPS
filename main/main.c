/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <common.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <uart.h>
#include <i2c.h>
#include "esp_timer.h"

int64_t millis(){
    int64_t result = esp_timer_get_time() / 1000;
    return result;
}

void delay(uint32_t ms){
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void app_main(void){
    printf("SPI Slave starting...\n");
    gpio_config_t config = {0};
    config.mode = GPIO_MODE_OUTPUT;
    config.pin_bit_mask = 1ULL << GPIO_NUM_2;
    ESP_ERROR_CHECK(gpio_config(&config));

    // xTaskCreate(TaskUART, "TaskUART/GPS read", 4096, NULL, 2, NULL);
    xTaskCreate(TaskI2C, "TaskMag/Temp read", 4096, NULL, 2, NULL);
    bool on = true;
    while(1){
        gpio_set_level(GPIO_NUM_2, on);
        on = !on;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}