idf_component_register(SRCS "main.c" "src/uart.c" "src/i2c.c" "src/common.c"
                    PRIV_REQUIRES esp_driver_uart esp_driver_i2c esp_driver_gpio esp_timer NMEA MPU6050 HMC5883
                    INCLUDE_DIRS "." "include")
