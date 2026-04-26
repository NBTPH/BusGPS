idf_component_register(SRCS "HMC5883_U.c"
                    PRIV_REQUIRES esp_driver_i2c LinearAlgebra
                    INCLUDE_DIRS "." "../../main/include")