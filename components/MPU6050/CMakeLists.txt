idf_component_register(SRCS "MPU6050.c"
                    PRIV_REQUIRES esp_driver_i2c
                    INCLUDE_DIRS "." "../../main/include" )