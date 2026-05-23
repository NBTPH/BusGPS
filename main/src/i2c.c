#include <i2c.h>
#include <math.h>

QueueHandle_t MPU6050_Queue = NULL;
QueueHandle_t HMC5883_Queue = NULL;

i2c_master_dev_handle_t mpu6050_i2c_dev = {0};
i2c_master_dev_handle_t hmc5883_i2c_dev = {0};

static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *i2c_mpu6050_dev_handle, i2c_master_dev_handle_t *i2c_hmc5883_dev_handle){
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
        .scl_speed_hz = 400000, //max frequency for MPU6050 and HMC5883L
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, i2c_mpu6050_dev_handle));

    dev_config.device_address = HMC5883_ADDRESS_MAG;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, i2c_hmc5883_dev_handle));

}

void TaskI2C(void *pvParameters){
    MPU6050_Queue = xQueueCreate(MPU6050_QUEUE_LENGTH, sizeof(MPU6050_Sensor_t));
    HMC5883_Queue = xQueueCreate(HMC5883_QUEUE_LENGTH, sizeof(HMC5883_Sensor_t));

    i2c_master_bus_handle_t bus_handle;
    i2c_master_init(&bus_handle, &mpu6050_i2c_dev, &hmc5883_i2c_dev);
    printf("I2C initialized successfully\n");

    if(!MPU6050_Init(&mpu6050_i2c_dev)){
        printf("MPU6050 failed to initialized\n");
        abort();
    }
    else{
        printf("MPU6050 initialized successfully\n");
    }

    if(!HMC5883_Init(&hmc5883_i2c_dev)){
        printf("HMC5883 failed to initialized\n");
        abort();
    }
    else{
        printf("HMC5883 initialized successfully\n");
    }
    
    MPU6050_Sensor_t mpu6050_data = {0};
    HMC5883_Sensor_t hmc5883_data = {0};
    // int64_t last_read_micros = micros();
    int64_t last_print_micros = micros();

    while(1){
        int64_t current_micros = micros();
        if(!MPU6050_is_Calibrating && !HMC5883_is_Calibrating){
            if(MPU6050_DataReady()){
                mpu6050_data.Timestamp = current_micros;

                float x, y, z;
                x = y = z = 0;
                
                getAccelData(&x, 
                            &y, 
                            &mpu6050_data.Accel.z);

                //remap to match with aviation standards, where X is North, Y is East (keep Z points up)
                mpu6050_data.Accel.x = -y;
                mpu6050_data.Accel.y = -x;

                getGyroData(&x, 
                            &y, 
                            &z);

                //same thing
                mpu6050_data.Gyro.x = -y;
                mpu6050_data.Gyro.y = -x;                    
                mpu6050_data.Gyro.z = -z;     

                if(xQueueSend(MPU6050_Queue, (void *)&mpu6050_data, 2) == errQUEUE_FULL){
                    printf("MPU6050 QUEUE FULL\r\n");
                }
            }
            else{
                // printf("MPU6050 DATA NOT READY\r\n");
            }

            if(HMC5883_DataReady()){
                float x, y, z;
                x = y = z = 0;
                
                getMagData(&x, 
                            &y, 
                            &z);

                //same thing
                hmc5883_data.x = -y;
                hmc5883_data.y = -x;
                hmc5883_data.z = -z;

                if(xQueueSend(HMC5883_Queue, (void *)&hmc5883_data, 2) == errQUEUE_FULL){
                    printf("HMC5883 QUEUE FULL\r\n");
                }
            }
            else{
                // printf("HMC5883 DATA NOT READY\r\n");
            }
        }
        else{
            delay(100);
        }
    }
}