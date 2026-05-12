idf_component_register(SRCS "main.c" "src/button.c" "src/uart.c" "src/i2c.c" "src/storage.c" "src/common.c" "src/ekf.c" "src/webpage.c"
                    PRIV_REQUIRES esp_driver_uart esp_driver_i2c esp_driver_gpio esp_timer nvs_flash esp_wifi esp_http_server NMEA MPU6050 HMC5883 LinearAlgebra TinyEKF
                    INCLUDE_DIRS "." "include"
                    EMBED_FILES "../webpage/coordinates.html" "../webpage/coordinates.js")
