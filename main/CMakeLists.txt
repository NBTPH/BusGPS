idf_component_register(SRCS "main.c" "src/button.c" "src/uart.c" "src/i2c.c" "src/storage.c" "src/common.c"
                    PRIV_REQUIRES nvs_flash esp_driver_uart esp_driver_i2c esp_driver_gpio esp_timer NMEA MPU6050 HMC5883
                    INCLUDE_DIRS "." "include")
