#include <i2c.h>

i2c_master_dev_handle_t i2c_MagDev = {0};
i2c_master_dev_handle_t i2c_AccelDev = {0};

static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *i2c_MagDev_handle, i2c_master_dev_handle_t *i2c_AccelDev_handle){
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LSM303_ADDRESS_ACCEL,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, i2c_AccelDev_handle));

    dev_config.device_address = LSM303_ADDRESS_MAG;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, i2c_MagDev_handle));
}

void TaskI2C(void *pvParameters){
    i2c_master_bus_handle_t bus_handle;
    i2c_master_init(&bus_handle, &i2c_MagDev, &i2c_AccelDev);
    printf("I2C initialized successfully\n");

    if(!LSM303_init(&i2c_MagDev, &i2c_AccelDev)){
        printf("LSM303 failed to initialized\n");
        abort();
    }
    else{
        printf("LSM303 initialized successfully\n");
    }

    float x, y, z;
    x = y = z = 0;

    while(1){
        get_Accel_Data(&x, &y, &z);
        printf("ACCEL X: %5f Y: %5f Z: %5f\r\n", x, y, z);
        if(get_Mag_Data(&x, &y, &z)){
            printf("MAG X: %5f Y: %5f Z: %5f\r\n\n\n", x, y, z);
        }
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}