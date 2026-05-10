#include <MPU6050_U.h>
#include <MPU6050.h>

i2c_master_dev_handle_t *mpu6050_dev_p = NULL;
mpu6050_t accel_bias = {0};
mpu6050_t gyro_bias = {0};

static void write8(const uint8_t reg_addr, const uint8_t data){
    uint8_t write_buff[2] = {reg_addr, data};
    ESP_ERROR_CHECK(i2c_master_transmit(*mpu6050_dev_p, write_buff, 2, I2C_TIMEOUT_MS));
}
static void read8(const uint8_t reg_addr, uint8_t *data){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*mpu6050_dev_p, &reg_addr, 1, data, 1, I2C_TIMEOUT_MS));
}

static void write_bytes(const uint8_t reg_addr, const uint8_t *data, size_t length){
    uint8_t *write_buff = malloc(sizeof(uint8_t) * (length + 1));
    write_buff[0] = reg_addr;
    memcpy(&write_buff[1], data, length);
    ESP_ERROR_CHECK(i2c_master_transmit(*mpu6050_dev_p, write_buff, length + 1, I2C_TIMEOUT_MS));
    free(write_buff);
}
static void read_bytes(const uint8_t reg_addr, uint8_t *data, size_t length){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*mpu6050_dev_p, &reg_addr, 1, data, length, I2C_TIMEOUT_MS));
}

uint8_t getSampleRateDivisor(void){
    uint8_t reg = 0;
    read8(MPU6050_SMPLRT_DIV, &reg);
    return reg;
}
bool setSampleRateDivisor(uint8_t divisor) {
    write8(MPU6050_SMPLRT_DIV, divisor);
    uint8_t check = 0;
    read8(MPU6050_SMPLRT_DIV, &check);
    return (check == divisor);
}

mpu6050_accel_range_t getAccelerometerRange(void){
    uint8_t reg = 0;
    read8(MPU6050_ACCEL_CONFIG, &reg);
    reg &= (3 << 3); //extract 4th and 5th bits
    reg = reg >> 3; //shifts back
    return reg;
}

bool setAccelerometerRange(mpu6050_accel_range_t range){
    uint8_t reg = 0;
    read8(MPU6050_ACCEL_CONFIG, &reg);
    reg &= ~(3 << 3); //clear 4th and 5th bits
    reg |= (range << 3); 
    write8(MPU6050_ACCEL_CONFIG, reg);

    uint8_t check = 0;
    read8(MPU6050_ACCEL_CONFIG, &check);
    return ((check >> 3) & 0x03) == (uint8_t)range;
}

mpu6050_gyro_range_t getGyroRange(void){
    uint8_t reg = 0;
    read8(MPU6050_GYRO_CONFIG, &reg);
    reg &= (3 << 3); //extract 4th and 5th bits
    reg = reg >> 3; //shifts back
    return reg;
}
bool setGyroRange(mpu6050_gyro_range_t range){
    uint8_t reg = 0;
    read8(MPU6050_GYRO_CONFIG, &reg);
    reg &= ~(3 << 3); //clear 4th and 5th bits
    reg |= (range << 3); //sets the bits
    write8(MPU6050_GYRO_CONFIG, reg);

    uint8_t check = 0;
    read8(MPU6050_GYRO_CONFIG, &check);
    return ((check >> 3) & 0x03) == (uint8_t)range;
}

bool setInterruptPinPolarity(bool active_low){
    uint8_t reg = 0;
    read8(MPU6050_INT_PIN_CONFIG, &reg);
    reg &= ~(1 << 7); //clear 7th bit
    if(active_low){
        reg |= (1 << 7); //set 7th bit if we want active low
    }
    write8(MPU6050_INT_PIN_CONFIG, reg);

    uint8_t check = 0;
    read8(MPU6050_INT_PIN_CONFIG, &check);
    return ((check >> 7) & 0x01) == active_low;
}
bool setInterruptPinLatch(bool held){
    uint8_t reg = 0;
    read8(MPU6050_INT_PIN_CONFIG, &reg);
    reg &= ~(1 << 5); //clear 5th bit
    if(held){
        reg |= (1 << 5); //set 5th bit if we want interrupt to held
    }
    write8(MPU6050_INT_PIN_CONFIG, reg);

    uint8_t check = 0;
    read8(MPU6050_INT_PIN_CONFIG, &check);
    return ((check >> 5) & 0x01) == held;
}
bool setInterruptClear(bool read_clear){
    uint8_t reg = 0;
    read8(MPU6050_INT_PIN_CONFIG, &reg);
    reg &= ~(1 << 4); //clear 5th bit
    if(read_clear){
        reg |= (1 << 4); //set 5th bit if we want interrupt to held
    }
    write8(MPU6050_INT_PIN_CONFIG, reg);

    uint8_t check = 0;
    read8(MPU6050_INT_PIN_CONFIG, &check);
    return ((check >> 4) & 0x01) == read_clear;
}
bool setFsyncSampleOutput(mpu6050_fsync_out_t fsync_output){
    uint8_t reg = 0;
    read8(MPU6050_CONFIG, &reg);
    reg &= ~(7 << 3); //clear 4th, 5th and 6th bits
    reg |= (fsync_output << 3); //sets the bits
    write8(MPU6050_CONFIG, reg);

    uint8_t check = 0;
    read8(MPU6050_CONFIG, &check);
    return ((check >> 3) & 0x07) == (uint8_t)fsync_output;
}

mpu6050_bandwidth_t getFilterBandwidth(void){
    uint8_t reg = 0;
    read8(MPU6050_CONFIG, &reg);
    reg &= 0x07; //extract first 3 bits
    return reg;
}
bool setFilterBandwidth(mpu6050_bandwidth_t bandwidth){
    uint8_t reg = 0;
    read8(MPU6050_CONFIG, &reg);
    reg &= ~(0x07); //clear the first 3 bits
    reg |= bandwidth; //set bits
    write8(MPU6050_CONFIG, reg);

    uint8_t check = 0;
    read8(MPU6050_CONFIG, &check);
    return (check & 0x07) == (uint8_t)bandwidth;
}


mpu6050_highpass_t getHighPassFilter(void){
    uint8_t reg = 0;
    read8(MPU6050_ACCEL_CONFIG, &reg);
    reg &= 3; //extract first 3 bits
    return reg;
}
bool setHighPassFilter(mpu6050_highpass_t bandwidth){
    uint8_t reg = 0;
    read8(MPU6050_ACCEL_CONFIG, &reg);
    reg &= ~(0x03); //clear the first 3 bits
    reg |= bandwidth; //set bits
    write8(MPU6050_ACCEL_CONFIG, reg);

    uint8_t check = 0;
    read8(MPU6050_ACCEL_CONFIG, &check);
    return (check & 0x07) == (uint8_t)bandwidth;
}

bool setDRDYInterruptPin(bool enable){
    uint8_t reg = 0;
    read8(MPU6050_INT_ENABLE, &reg);
    reg &= ~(0x01); //clear 1st bit
    if(enable){
        reg |= 1; //set 1st bit if we want to enable DRDY interrupt
    }
    write8(MPU6050_INT_ENABLE, reg);

    uint8_t check = 0;
    read8(MPU6050_INT_ENABLE, &check);
    return (check & 0x01) == enable;
}

bool MPU6050_DataReady(void){
    uint8_t reg = 0;
    read8(MPU6050_INT_STATUS, &reg);
    reg &= 0x01; //extract first bit
    return reg;
}

void setMotionInterrupt(bool active);
void setMotionDetectionThreshold(uint8_t thr);
void setMotionDetectionDuration(uint8_t dur);
bool getMotionInterruptStatus(void);

mpu6050_fsync_out_t getFsyncSampleOutput(void);

bool setI2CBypass(bool bypass){
    uint8_t reg = 0;
    read8(MPU6050_INT_PIN_CONFIG, &reg);
    reg &= ~(1 << 1); //clear 2nd bit
    if(bypass)
        reg |= (1 << 1); //set 2nd bit if we want to enable I2C bypass
    write8(MPU6050_INT_PIN_CONFIG, reg);

    uint8_t check = 0;
    read8(MPU6050_INT_PIN_CONFIG, &check);
    return ((check >> 1) & 0x01) == bypass;
}

void setClock(mpu6050_clock_select_t);
mpu6050_clock_select_t getClock(void);

bool enableSleep(bool enable);
bool enableCycle(bool enable);

void setCycleRate(mpu6050_cycle_rate_t rate);
mpu6050_cycle_rate_t getCycleRate(void);

bool setGyroStandby(bool xAxisStandby, bool yAxisStandby, bool zAxisStandby);
bool setAccelerometerStandby(bool xAxisStandby, bool yAxisStandby, bool zAxisStandby);
bool setTemperatureStandby(bool enable);

void resetRegisters(void){
    write8(MPU6050_PWR_MGMT_1, 1 << 7);

    uint8_t reg = 0;
    do {
        delay(1);
        read8(MPU6050_PWR_MGMT_1, &reg);
    }while (reg & (1 << 7));

    delay(20);
}
void resetSignalPath(void){
    write8(MPU6050_SIGNAL_PATH_RESET, 7);
    delay(20);
}

void getAccelRawData(int16_t *x, int16_t *y, int16_t *z){
    uint8_t reg[6] = {0};
    read_bytes(MPU6050_ACCEL_OUT, reg, 6);
    *x = (int16_t)((reg[0] << 8) | reg[1]);
    *y = (int16_t)((reg[2] << 8) | reg[3]);
    *z = (int16_t)((reg[4] << 8) | reg[5]);
    // printf("ACCEL RAW X: %d Y: %d Z: %d\r\n", *x, *y, *z);
}
void getGyroRawData(int16_t *x, int16_t *y, int16_t *z){
    uint8_t reg[6] = {0};
    read_bytes(MPU6050_GYRO_OUT, reg, 6);
    *x = (int16_t)((reg[0] << 8) | reg[1]);
    *y = (int16_t)((reg[2] << 8) | reg[3]);
    *z = (int16_t)((reg[4] << 8) | reg[5]);
    // printf("GYRO RAW X: %d Y: %d Z: %d\r\n\n\n", *x, *y, *z);
}
int16_t getTempRawData(void){
    uint8_t reg[2] = {0};
    read_bytes(MPU6050_TEMP_H, reg, 2);
    return (int16_t)((reg[0] << 8) | reg[1]);
}

void getTempSensor(void);
void getAccelData(float *x, float *y, float *z){
    //get LSB sensitivity
    mpu6050_accel_range_t accel_range = getAccelerometerRange();
    float accel_scale = 1;
    switch (accel_range){
        case MPU6050_RANGE_16_G:
            accel_scale = 2048;
            break;
        case MPU6050_RANGE_8_G:
            accel_scale = 4096;
            break;
        case MPU6050_RANGE_4_G:
            accel_scale = 8192;
            break;
        case MPU6050_RANGE_2_G:
            accel_scale = 16384;
            break;
    }
    //get raw data
    int16_t x_raw, y_raw ,z_raw;
    x_raw = y_raw = z_raw = 0;
    getAccelRawData(&x_raw, &y_raw, &z_raw);
    //scale raw data
    *x = ((float)x_raw - accel_bias.x) / accel_scale;
    *y = ((float)y_raw - accel_bias.y) / accel_scale;
    *z = ((float)z_raw - accel_bias.z) / accel_scale;
}
void getGyroData(float *x, float *y, float *z){
    //get LSB sensitivity
    mpu6050_gyro_range_t gyro_range = getGyroRange();
    float gyro_scale = 1;
    switch (gyro_range){
        case MPU6050_RANGE_250_DEG:
            gyro_scale = 131;
            break;
        case MPU6050_RANGE_500_DEG:
            gyro_scale = 65.5;
            break;
        case MPU6050_RANGE_1000_DEG:
            gyro_scale = 32.8;
            break;
        case MPU6050_RANGE_2000_DEG:
            gyro_scale = 16.4;
            break;
    }
    
    //get raw data
    int16_t x_raw, y_raw ,z_raw;
    x_raw = y_raw = z_raw = 0;
    getGyroRawData(&x_raw, &y_raw, &z_raw);
    
    //scale raw data
    *x = ((float)x_raw - gyro_bias.x) / gyro_scale;
    *y = ((float)y_raw - gyro_bias.y) / gyro_scale;
    *z = ((float)z_raw - gyro_bias.z) / gyro_scale;
}

bool MPU6050_is_Calibrating = false;
void MPU6050_Calibration(uint32_t samples_num){
    if(!MPU6050_is_Calibrating){
        MPU6050_is_Calibrating = true;
        delay(5); //wait until all tasks has noticed the state change
        printf("=========== MPU6050 Calibrations begin ===========\r\n");
        int32_t accum_acc_x = 0, accum_acc_y = 0, accum_acc_z = 0;
        int32_t accum_gyro_x = 0, accum_gyro_y = 0, accum_gyro_z = 0;
        for(uint32_t i = 0; i < samples_num; i++){
            while(!MPU6050_DataReady()); //wait until data ready

            int16_t x_raw = 0, y_raw = 0, z_raw = 0;
            getAccelRawData(&x_raw, &y_raw, &z_raw);
            accum_acc_x += x_raw;
            accum_acc_y += y_raw;
            accum_acc_z += z_raw;

            x_raw = y_raw = z_raw = 0;
            getGyroRawData(&x_raw, &y_raw, &z_raw);
            accum_gyro_x += x_raw;
            accum_gyro_y += y_raw;
            accum_gyro_z += z_raw;

            if(((i + 1) % 100) == 0){
                printf("Calibrating %ld/%ld samples\r\n", i + 1, samples_num);
            }
            delay(1);
        }

        mpu6050_accel_range_t accel_range = getAccelerometerRange();
        float accel_scale = 1;
        switch (accel_range){
            case MPU6050_RANGE_16_G:
                accel_scale = 2048;
                break;
            case MPU6050_RANGE_8_G:
                accel_scale = 4096;
                break;
            case MPU6050_RANGE_4_G:
                accel_scale = 8192;
                break;
            case MPU6050_RANGE_2_G:
                accel_scale = 16384;
                break;
        }

        accel_bias.x = (float)accum_acc_x / (float)samples_num;
        accel_bias.y = (float)accum_acc_y / (float)samples_num;
        accel_bias.z = ((float)accum_acc_z / (float)samples_num) - accel_scale; //calibrate when MPU is level
        
        gyro_bias.x = (float)accum_gyro_x / (float)samples_num;
        gyro_bias.y = (float)accum_gyro_y / (float)samples_num;
        gyro_bias.z = (float)accum_gyro_z / (float)samples_num;

        MPU6050_is_Calibrating = false;
        
        printf("\n=========== MPU6050 Calibrations Completed ===========\r\n");
        printf("Calibrated results\r\n\n"

                "Accelerometer bias X: %5f\r\n"
                "Accelerometer bias Y: %5f\r\n"
                "Accelerometer bias Z: %5f\r\n\n"
                
                "Gyroscope bias X: %5f\r\n"
                "Gyroscope bias Y: %5f\r\n"
                "Gyroscope bias Z: %5f\r\n"
                , accel_bias.x, accel_bias.y, accel_bias.z, gyro_bias.x, gyro_bias.y, gyro_bias.z);

        if(!flash_write("accel_bias", (void*)&accel_bias, sizeof(mpu6050_t))){
            printf("Failed writing accelerometer calibrate data!\r\n");
        }
        if(!flash_write("gyro_bias", (void*)&gyro_bias, sizeof(mpu6050_t))){
            printf("Failed writing Gyroscope calibrate data!\r\n");
        }
    }
}

bool MPU6050_Init(i2c_master_dev_handle_t *input_mpu6050_dev){
    if(input_mpu6050_dev == NULL){
        printf("Error: MPU6050 I2C dev handler not provided\r\n");
        return false;
    }
    mpu6050_dev_p = input_mpu6050_dev;

    uint8_t reg = 0;
    read8(MPU6050_WHO_AM_I, &reg);
    if(reg != MPU6050_DEVICE_ID){
        printf("Error: MPU6050 failed WHO AM I test\r\n");
        return false;
    }
    
    resetRegisters(); //this will put device into sleep mode
    resetSignalPath();

    write8(MPU6050_PWR_MGMT_1, 0x00); //we have to write to PWR MGMT 1 reg to wake it up from sleep after reset

    if(!setSampleRateDivisor(0)) 
        printf("Error: Failed to set Sample Rate Divisor\r\n");

    if(!setFilterBandwidth(MPU6050_BAND_21_HZ)) 
        printf("Error: Failed to set Filter Bandwidth\r\n");
    
    if(!setGyroRange(MPU6050_RANGE_500_DEG)) 
        printf("Error: Failed to set Gyroscope Range\r\n");

    if(!setAccelerometerRange(MPU6050_RANGE_2_G)) 
        printf("Error: Failed to set Accelerometer Range\r\n");

    if(!setInterruptPinPolarity(false)) 
        printf("Error: Failed to set Interrupt Pin Polarity\r\n");
    
    if(!setInterruptPinLatch(false)) 
        printf("Error: Failed to set Interrupt Pin Latch\r\n");
    
    if(!setInterruptClear(false)) 
        printf("Error: Failed to set Interrupt Clear behavior\r\n");
    
    if(!setDRDYInterruptPin(true)) 
        printf("Error: Failed to configure Data Ready Interrupt\r\n");

    if(!setI2CBypass(true)) //IMPORTANT for module such as the GY-87 where Magnetometer are connected to AUX I2C bus of MPU6050
        printf("Error: Failed to configure I2C Bypass\r\n");

    if(!flash_read("accel_bias", (void*)&accel_bias, sizeof(mpu6050_t))){
        printf("\r\nAccelerometer calibrate data loading failed\r\n");
    }
    else{
        printf("\r\nAccelerometer loaded calibrated results:\r\n"
        "Accelerometer bias X: %5f\r\n"
        "Accelerometer bias Y: %5f\r\n"
        "Accelerometer bias Z: %5f\r\n", accel_bias.x, accel_bias.y, accel_bias.z);
    }

    if(!flash_read("gyro_bias", (void*)&gyro_bias, sizeof(mpu6050_t))){
        printf("\r\nGyroscope calibrate data loading failed\r\n");
    }
    else{
        printf("\r\nGyroscope loaded calibrated results:\r\n"
        "Gyroscope bias X: %5f\r\n"
        "Gyroscope bias Y: %5f\r\n"
        "Gyroscope bias Z: %5f\r\n", gyro_bias.x, gyro_bias.y, gyro_bias.z);
    }
    return true;
}