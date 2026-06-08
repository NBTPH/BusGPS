#include <i2c.h>
#include <math.h>

QueueHandle_t MPU6050_Queue = NULL;
QueueHandle_t HMC5883_Queue = NULL;
QueueHandle_t DHT20_Queue = NULL;

i2c_master_dev_handle_t mpu6050_i2c_dev = {0};
i2c_master_dev_handle_t hmc5883_i2c_dev = {0};
i2c_master_dev_handle_t dht20_i2c_dev = {0};

static void GY87_i2c_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *i2c_mpu6050_dev_handle, i2c_master_dev_handle_t *i2c_hmc5883_dev_handle){
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

static void DHT20_i2c_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *i2c_dht20_dev_handle){
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_1,
        .sda_io_num = SDA_PIN2,
        .scl_io_num = SCL_PIN2,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DHT20_I2C_ADDRESS,
        .scl_speed_hz = 200000, //slower freg
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, i2c_dht20_dev_handle));
}

void TaskGY87(void *pvParameters){
    MPU6050_Queue = xQueueCreate(MPU6050_QUEUE_LENGTH, sizeof(MPU6050_Sensor_t));
    HMC5883_Queue = xQueueCreate(HMC5883_QUEUE_LENGTH, sizeof(HMC5883_Sensor_t));

    i2c_master_bus_handle_t gy87_bus_handle;
    GY87_i2c_init(&gy87_bus_handle, &mpu6050_i2c_dev, &hmc5883_i2c_dev);
    printf("GY87 I2C initialized successfully\n");

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
    // int64_t last_print_micros = micros();

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

void TaskDHT20(void *pvParameters){
    DHT20_Queue = xQueueCreate(DHT20_QUEUE_LENGTH, sizeof(DHT20_Sensor_t));

    i2c_master_bus_handle_t dht20_bus_handle;
    DHT20_i2c_init(&dht20_bus_handle, &dht20_i2c_dev);
    printf("DHT20 I2C initialized successfully\n");

    DHT20_Sensor_t dht20_data = {0};
    bool dht20_detected = false;

    while(1){
        int64_t current_millis = millis();
        if(!dht20_detected){
            if(DHT20_Init(&dht20_i2c_dev)){
                dht20_detected = true;
                printf("DHT20 initialized succesfully\n");
                continue; //loop back so we don't have to wait
            }
            printf("DHT20 failed to initialized or not conntected\n");
            delay(2000);
        }
        else{
            delay(100); //fairly slow sensor
            int read_return = DHT20_ReadData_NonBlocking();
            switch(read_return){
                case 0: //success reading data
                    DHT20_GetData(&dht20_data.humi, &dht20_data.temp);

                    if(xQueueSend(DHT20_Queue, (void *)&dht20_data, 2) == errQUEUE_FULL){
                        printf("DHT20 QUEUE FULL\r\n");
                    }

                    break;

                case 1: //busy reading data, just loop back to check until complete
                    continue; 
                    break;

                case 2: //timed out reading data
                    dht20_detected = false; //reinitialize dht20
                    printf("DHT20 failed to read, device might not be connected\n");
                    dht20_data.humi = dht20_data.temp = NAN;

                    if(xQueueSend(DHT20_Queue, (void *)&dht20_data, 2) == errQUEUE_FULL){
                        printf("DHT20 QUEUE FULL\r\n");
                    }
                    break;

                default:
                    continue;
                    break;
            }
        }
    }
}