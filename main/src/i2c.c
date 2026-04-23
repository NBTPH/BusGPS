#include <i2c.h>
#include <math.h>

i2c_master_dev_handle_t mpu6050_i2c_dev = {0};

static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *i2c_dev_handle){
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
        .device_address = MPU6050_I2CADDR_DEFAULT,
        .scl_speed_hz = 400000, //max frequency for MPU6050
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, i2c_dev_handle));
}

void TaskI2C(void *pvParameters){
    i2c_master_bus_handle_t bus_handle;
    i2c_master_init(&bus_handle, &mpu6050_i2c_dev);
    printf("I2C initialized successfully\n");

    if(!MPU6050_Init(&mpu6050_i2c_dev)){
        printf("MPU6050 failed to initialized\n");
        abort();
    }
    else{
        printf("MPU6050 initialized successfully\n");
    }

    float x, y, z;
    x = y = z = 0;

    while(1){
        int64_t start = millis();
        getAccelerometerData(&x, &y, &z);
        // printf("ACCEL X: %5f Y: %5f Z: %5f\r\n", x, y, z);
        // getGyroData(&x, &y, &z);
        // printf("GYRO X: %5f Y: %5f Z: %5f\r\n", x, y, z);
        int64_t interval = millis() - start;
        printf("ACCEL X: %5f Y: %5f Z: %5f\r\n", x, y, z);
        printf("Time taken %lld ms\r\n\n\n", interval);
        // float x_temp, y_temp, z_temp;
        // x_temp = y_temp = z_temp = 0;
        // if(get_Mag_Data(&x_temp, &y_temp, &z_temp)){
        //     printf("MAG X: %5f Y: %5f Z: %5f\r\n", x_temp, y_temp, z_temp);

        //     x = -x_temp;
        //     y = -y_temp;
        //     z = z_temp;

        //     float headingRad = atan2(y, x); //calculate heading, the output is the angle value in radiant of North relative to X axis
        //     float headingDeg = (headingRad * 180) / M_PI;
        //     float declinationAngle = -0.64166666666667; //declination angle in HCMC as of April 2026

        //     headingDeg += declinationAngle;

        //     if (headingDeg < 0)    headingDeg += 360;

        //     printf("HEADING DEGREE: %5f\r\n\n\n", headingDeg);
        // }
        delay(250);
    }
}