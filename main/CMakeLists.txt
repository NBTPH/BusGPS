idf_component_register(SRCS "main.c" "src/uart.c" "src/i2c.c"
                    PRIV_REQUIRES esp_driver_uart esp_driver_i2c esp_driver_gpio esp_timer NMEA LSM303 
                    INCLUDE_DIRS "." "include")
