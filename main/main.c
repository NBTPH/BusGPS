/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <common.h>
#include <GPS.h>

void app_main(void){
    printf("SPI Slave starting...\n");
    gpio_config_t config = {0};
    config.mode = GPIO_MODE_OUTPUT;
    config.pin_bit_mask = 1ULL << GPIO_NUM_2;
    ESP_ERROR_CHECK(gpio_config(&config));

    xTaskCreate(TaskGPS, "rf24 SPI read", 4096, NULL, 2, NULL);
    bool on = true;
    while(1){
        gpio_set_level(GPIO_NUM_2, on);
        on = !on;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}