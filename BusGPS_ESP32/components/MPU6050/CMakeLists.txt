idf_component_register(SRCS "MPU6050_U.c"
                    PRIV_REQUIRES nvs_flash esp_driver_i2c
                    INCLUDE_DIRS "." "../../main/include")