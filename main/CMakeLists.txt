idf_component_register(SRCS "main.c" "src/uart.c"
                    PRIV_REQUIRES esp_driver_uart esp_driver_gpio esp_timer NMEA
                    INCLUDE_DIRS "." "include")
