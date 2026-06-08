#include <DHT20.h>
#include "freertos/FreeRTOS.h"

i2c_master_dev_handle_t *dht20_dev_p = NULL;
float _humi = 0;
float _temp = 0;

static void write8(const uint8_t data){ //we won't use ESP_ERROR_CHECK because we are checking connectivity with timeout, and DHT20 takes too long for timeout
    i2c_master_transmit(*dht20_dev_p, &data, 1, pdMS_TO_TICKS(10));
}
static void read8(uint8_t *data){
    i2c_master_receive(*dht20_dev_p, data, 1, pdMS_TO_TICKS(10));
}

static void write_bytes(const uint8_t *data, size_t length){
    i2c_master_transmit(*dht20_dev_p, data, length, pdMS_TO_TICKS(10));
}
static void read_bytes(uint8_t *data, size_t length){
    i2c_master_receive(*dht20_dev_p, data, length, pdMS_TO_TICKS(10));
}

static uint8_t DHT20_CRC(uint8_t *value, size_t length){
    uint8_t crc = 0xFF; //inital value
    for(size_t i = 0; i < length; i++){
        crc ^= value[i];
        for(uint8_t j = 0; j < 8; j++){
            if(crc & (1 << 7)){// if MSB is 1
                crc <<= 1; //shift to left by one
                crc ^= 0x31; //2^8 + 2^5 + 2^4 + 1 = 49 = 0x31
            }
            else{
                crc <<= 1; //just left until 8th bit is one
            }
        }
    }
    debug_printf("[DHT20_CRC] CRC result: %x\r\n", crc);
    return crc;
}

static void DHT20_ResetRegister(uint8_t reg){
    uint8_t buff[3] = {0};
    buff[0] = reg;
    write_bytes(buff, 3);
    delay(5);
    debug_printf("[DHT20_Init] Calling reset registers \r\n");
    read_bytes(buff, 3);
    delay(10);

    buff[0] = 0xB0 | reg;
    write_bytes(buff, 3);
    delay(5);
}

uint8_t DHT20_GetStatus(){ //literally just send the a read address then reads back 1 byte
    uint8_t status = (1 << 7); //initialized with 7th bit set, so we can detect when it's idle
    read8(&status);
    debug_printf("[DHT20] Status: %x\r\n", status);
    return status;
}

bool DHT20_Init(i2c_master_dev_handle_t *input_dht20_dev){
    if(input_dht20_dev == NULL){
        printf("Error: DHT20 I2C dev handler not provided\r\n");
        return false;
    }
    dht20_dev_p = input_dht20_dev;
    delay(5);

    uint8_t status = 0;
    debug_printf("[DHT20_Init] Calling status \r\n");
    for(uint8_t i = 0; i < 10; i++){
        status = DHT20_GetStatus();
        if((status & 0x18) != 0x18){
            if(i == 9){
                return false;
            }
            DHT20_ResetRegister(0x1B);
            DHT20_ResetRegister(0x1C);
            DHT20_ResetRegister(0x1E);
        }
        else{
            break;
        }
    }

    delay(10); //wait 10ms before we can start sending data;
    return true;
}

void DHT20_TriggerMeasure(){ //sends triggermeasure command
    uint8_t buff[8] = {0};

    buff[0] = 0xAC;
    buff[1] = 0x33;
    buff[2] = 0x00;
    write_bytes(buff, 3); //Trigger measurement
}

bool DHT20_GetRawData(uint32_t *raw_humi, uint32_t *raw_temp){
    *raw_humi = *raw_temp = 0;

    uint8_t status = 0;
    status = DHT20_GetStatus(); //if the read status word Bit [7] is 1 it's still busy
    if(status & (1 << 7)){ //if the read status word Bit [7] is still 1 it's still busy
        printf("[DHT20_GetRawData] Device still busy\r\n");
        return false;
    }

    uint8_t buff[7] = {0}; //get status, humi, temp and crc data (because GetStatus didn't retain the pointer, when we start a new transaction we have to read 7 bytes in series)
    read_bytes(buff, 7);
    debug_printf("[DHT20_GetRawData] [%x, %x, %x, %x, %x, %x, %x]\r\n", buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6]);
    if(DHT20_CRC(buff, 6) != buff[6]){ //crc check on data and status
        debug_printf("[DHT20_GetRawData] Error: CRC check failed! Raw CRC: %x\r\n", buff[6]);
        return false;
    }

    *raw_humi = buff[1];
    *raw_humi <<= 8; //shifts 8
    *raw_humi |= (uint32_t)buff[2]; //puts another 8 bits into the empty shifted space
    *raw_humi <<= 4; //shifts 4
    *raw_humi |= (uint32_t)(buff[3] >> 4); //puts the remaining 4 bits into the empty shifted space

    *raw_temp = (uint32_t)(buff[3] & 0x0F); //get the first 4 bits
    *raw_temp <<= 8; //shifts 8
    *raw_temp |= (uint32_t)buff[4]; //puts another 8 bits into the empty shifted space
    *raw_temp <<= 8; //shifts 8
    *raw_temp |= (uint32_t)buff[5]; //puts the remaining 8 bits into the empty shifted space

    return true;
}

bool DHT20_ReadData(){
    DHT20_TriggerMeasure();
    delay(80);

    uint32_t raw_humi = 0, raw_temp = 0;
    uint8_t GetRawData_count = 0;
    while(!DHT20_GetRawData(&raw_humi, &raw_temp)){
        if(GetRawData_count > 50){
            return false;
        }
        GetRawData_count++;
        delay(1);
    }

    _humi = (float)raw_humi * 9.5367431640625e-5f; //divided to 1048576.0 * 100%;
    _temp = (float)raw_temp * 1.9073486328125e-4f - 50; //divided to 1048576.0 * 200 - 50;
    return true;
}

int64_t TriggerMeasure_timestamp = -1;
int64_t GetRawData_timestamp = -1; //incase getting raw data fails, 0 means success, 1 means busy, 2 means get data fails before time out
int DHT20_ReadData_NonBlocking(){ //returns immediately, used in a loop
    uint32_t raw_humi = 0, raw_temp = 0;

    if(TriggerMeasure_timestamp < 0){ //if TriggerMeasure_timestamp is negative, that means TriggerMeasure hasnt been sent out
        DHT20_TriggerMeasure();
        TriggerMeasure_timestamp = millis();
        return 1;
    }

    else if(millis() - TriggerMeasure_timestamp < 80){ //if TriggerMeasure_timestamp is not negative, then evalutate if weve waited enough
        return 1;
    }

    else if(!DHT20_GetRawData(&raw_humi, &raw_temp)){ //if we've waited enough, get raw data
        if(GetRawData_timestamp < 0){ //if this is the first time get data is called, set the timestamp
            GetRawData_timestamp =  millis();
        }
        else if(millis() - GetRawData_timestamp > 50){ //if the get data time has timed out, something is wrong
            TriggerMeasure_timestamp = -1; //clear the states
            GetRawData_timestamp = -1;
            return 2;
        }
        return 1;
    }

    TriggerMeasure_timestamp = -1;
    GetRawData_timestamp = -1;
    _humi = (float)raw_humi * 9.5367431640625e-5f; //divided to 1048576.0 * 100%;
    _temp = (float)raw_temp * 1.9073486328125e-4f - 50; //divided to 1048576.0 * 200 - 50;
    return 0;
}

void DHT20_GetData(float *ptr_humi, float *ptr_temp){
    if(ptr_humi == NULL || ptr_temp == NULL){
        return;
    }
    *ptr_humi = _humi;
    *ptr_temp = _temp;
}

float DHT20_GetHumidity(){
    return _humi;
}

float DHT20_GetTemperature(){
    return _humi;
}