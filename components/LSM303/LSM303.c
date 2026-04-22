#include <LSM303.h>

i2c_master_dev_handle_t *i2c_MagDev_p = NULL;
i2c_master_dev_handle_t *i2c_AccelDev_p = NULL;
lsm303MagGain magGain;

float _lsm303Accel_MG_LSB     = 0.001F;   // 1, 2, 4 or 12 mg per lsb
float _lsm303Mag_Gauss_LSB_XY = 1100.0F;  // Varies with gain
float _lsm303Mag_Gauss_LSB_Z  = 980.0F;   // Varies with gain

//I2C write, read functions

void write8_mag(const uint8_t reg_addr, const uint8_t data){
    uint8_t write_buffer[2] = {reg_addr, data};
    ESP_ERROR_CHECK(i2c_master_transmit(*i2c_MagDev_p, write_buffer, 2, 250));
}
void read8_mag(const uint8_t reg_addr, uint8_t *data){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*i2c_MagDev_p, &reg_addr, 1, data, 1, 250));
}

void write8_accel(const uint8_t reg_addr, const uint8_t data){
    uint8_t write_buffer[2] = {reg_addr, data};
    ESP_ERROR_CHECK(i2c_master_transmit(*i2c_AccelDev_p, write_buffer, 2, 250));
}
void read8_accel(const uint8_t reg_addr, uint8_t *data){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*i2c_AccelDev_p, &reg_addr, 1, data, 1, 250));
}

void write_bytes_mag(const uint8_t reg_addr, const uint8_t *data, size_t length){
    uint8_t *write_buffer = malloc(sizeof(uint8_t) * (length + 1));
    write_buffer[0] = reg_addr;
    memcpy(&write_buffer[1], data, length);
    ESP_ERROR_CHECK(i2c_master_transmit(*i2c_MagDev_p, write_buffer, length + 1, 250));
    free(write_buffer);
}
void read_bytes_mag(const uint8_t reg_addr, uint8_t *data, size_t length){
    const uint8_t reg_addr_auto_incr = reg_addr | 0x80;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*i2c_MagDev_p, &reg_addr_auto_incr, 1, data, length, 250));
}

void write_bytes_accel(const uint8_t reg_addr, const uint8_t *data, size_t length){
    uint8_t *write_buffer = malloc(sizeof(uint8_t) * (length + 1));
    write_buffer[0] = reg_addr;
    memcpy(&write_buffer[1], data, length);
    ESP_ERROR_CHECK(i2c_master_transmit(*i2c_AccelDev_p, write_buffer, length + 1, 250));
    free(write_buffer);
}
void read_bytes_accel(const uint8_t reg_addr, uint8_t *data, size_t length){
    const uint8_t reg_addr_auto_incr = reg_addr | 0x80;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*i2c_AccelDev_p, &reg_addr_auto_incr, 1, data, length, 250));
}

//Accelerometers functions

void set_Accel_Range(lsm303_accel_range_t new_range) {
    uint8_t reg = 0;
    read8_accel(LSM303_REGISTER_ACCEL_CTRL_REG4_A, &reg); //get current register value
    reg &= ~(3 << 4); //clear 5th and 6th bits
    reg |= (new_range << 4); //set 5th and 6th bits
    write8_accel(LSM303_REGISTER_ACCEL_CTRL_REG4_A, reg); //rewrite it back
}

lsm303_accel_range_t get_Accel_Range() {
    uint8_t reg = 0;
    read8_accel(LSM303_REGISTER_ACCEL_CTRL_REG4_A, &reg); //get current register value
    reg &= (3 << 4); //extract 5th and 6th bits
    return (reg >> 4); //shift back
}

void set_Accel_Mode(lsm303_accel_mode_t new_range) {
    uint8_t reg[2] = {0};
    read8_accel(LSM303_REGISTER_ACCEL_CTRL_REG1_A, &reg[0]); //get current register value
    read8_accel(LSM303_REGISTER_ACCEL_CTRL_REG4_A, &reg[1]); //get current register value
    reg[0] &= ~(1 << 3); //clear LP bit
    reg[1] &= ~(1 << 3); //clear HR bit
    switch (new_range){
    case LSM303_MODE_NORMAL:
        //don't need to do anything, just write back with cleared bits for LP and HR
        break;
    case LSM303_MODE_HIGH_RESOLUTION: //set high resolution bit
        reg[1] |= (1 << 3);
        break;
    case LSM303_MODE_LOW_POWER: //set low power bit
        reg[0] |= (1 << 3);
        break;
    default:
        break;
    }
    write8_accel(LSM303_REGISTER_ACCEL_CTRL_REG1_A, reg[0]);
    write8_accel(LSM303_REGISTER_ACCEL_CTRL_REG4_A, reg[1]);
}

lsm303_accel_mode_t get_Accel_Mode(){
    uint8_t reg[2] = {0};
    read8_accel(LSM303_REGISTER_ACCEL_CTRL_REG1_A, &reg[0]); //get current register value
    read8_accel(LSM303_REGISTER_ACCEL_CTRL_REG4_A, &reg[1]); //get current register value
    reg[0] &= (1 << 3); //extract LP bit
    reg[0] = (reg[0] >> 3); //shifts back
    reg[1] &= (1 << 3); //extract HR bit
    reg[1] = (reg[1] >> 3); //shifts back
    if(reg[0]){ //if LP bit is set, it is in low power
        return LSM303_MODE_LOW_POWER;
    }
    else if(reg[1]){ //if HR bit is set, it is in high resolution
        return LSM303_MODE_HIGH_RESOLUTION;
    }
    else{ //if none of the bits are set, it is in normal mode
        return LSM303_MODE_NORMAL;
    }
}

float get_Accel_LSB(lsm303_accel_mode_t mode) {
    float lsb = 0;
    lsm303_accel_range_t range = get_Accel_Range();
    if (mode == LSM303_MODE_NORMAL) {
        switch (range) {
        case LSM303_RANGE_2G:
            lsb = 0.0039;
            break;
        case LSM303_RANGE_4G:
            lsb = 0.00782;
            break;
        case LSM303_RANGE_8G:
            lsb = 0.01563;
            break;
        case LSM303_RANGE_16G:
            lsb = 0.0469;
            break;
        }
    }

    else if (mode == LSM303_MODE_HIGH_RESOLUTION) {
        switch (range) {
        case LSM303_RANGE_2G:
            lsb = 0.00098;
            break;
        case LSM303_RANGE_4G:
            lsb = 0.00195;
            break;
        case LSM303_RANGE_8G:
            lsb = 0.0039;
            break;
        case LSM303_RANGE_16G:
            lsb = 0.01172;
            break;
        }
    } else if (mode == LSM303_MODE_LOW_POWER) {
        switch (range) {
        case LSM303_RANGE_2G:
            lsb = 0.01563;
            break;
        case LSM303_RANGE_4G:
            lsb = 0.03126;
            break;
        case LSM303_RANGE_8G:
            lsb = 0.06252;
            break;
        case LSM303_RANGE_16G:
            lsb = 0.18758;
            break;
        }
    }
    return lsb;
}

uint8_t get_Accel_Shift(lsm303_accel_mode_t mode) {
    uint8_t shift = 0;
    switch (mode) {
    case LSM303_MODE_HIGH_RESOLUTION:
        shift = 4;
        break;
    case LSM303_MODE_NORMAL:
        shift = 6;
        break;
    case LSM303_MODE_LOW_POWER:
        shift = 8;
        break;
    }
    return shift;
}

void get_Accel_Raw_Data(int16_t *x, int16_t *y, int16_t *z){
    uint8_t accel_out[6] = {0};
    read_bytes_accel(LSM303_REGISTER_ACCEL_OUT_X_L_A, accel_out, 6);
    *x = (int16_t)(accel_out[0] | (accel_out[1] << 8));
    *y = (int16_t)(accel_out[2] | (accel_out[3] << 8));
    *z = (int16_t)(accel_out[4] | (accel_out[5] << 8));
}

bool get_Accel_Data(float *x, float *y, float *z){
    int16_t x_raw, y_raw, z_raw;
    x_raw = y_raw = z_raw = 0;
    get_Accel_Raw_Data(&x_raw, &y_raw, &z_raw);

    lsm303_accel_mode_t mode = get_Accel_Mode();
    float lsb = get_Accel_LSB(mode);
    uint8_t shift = get_Accel_Shift(mode);

    *x = (float)(x_raw >> shift) * lsb;
    *y = (float)(y_raw >> shift) * lsb;
    *z = (float)(z_raw >> shift) * lsb;

    return true;
}

//Magnetometer functions

void setMagGain(lsm303MagGain gain){
  write8_mag(LSM303_REGISTER_MAG_CRB_REG_M, (uint8_t)gain);

  magGain = gain;

  switch(gain)
  {
    case LSM303_MAGGAIN_1_3:
      _lsm303Mag_Gauss_LSB_XY = 1100;
      _lsm303Mag_Gauss_LSB_Z  = 980;
      break;
    case LSM303_MAGGAIN_1_9:
      _lsm303Mag_Gauss_LSB_XY = 855;
      _lsm303Mag_Gauss_LSB_Z  = 760;
      break;
    case LSM303_MAGGAIN_2_5:
      _lsm303Mag_Gauss_LSB_XY = 670;
      _lsm303Mag_Gauss_LSB_Z  = 600;
      break;
    case LSM303_MAGGAIN_4_0:
      _lsm303Mag_Gauss_LSB_XY = 450;
      _lsm303Mag_Gauss_LSB_Z  = 400;
      break;
    case LSM303_MAGGAIN_4_7:
      _lsm303Mag_Gauss_LSB_XY = 400;
      _lsm303Mag_Gauss_LSB_Z  = 355;
      break;
    case LSM303_MAGGAIN_5_6:
      _lsm303Mag_Gauss_LSB_XY = 330;
      _lsm303Mag_Gauss_LSB_Z  = 295;
      break;
    case LSM303_MAGGAIN_8_1:
      _lsm303Mag_Gauss_LSB_XY = 230;
      _lsm303Mag_Gauss_LSB_Z  = 205;
      break;
  }
}

void setMagRate(lsm303MagRate rate){
    uint8_t current_reg = 0;
    read8_mag(LSM303_REGISTER_MAG_CRA_REG_M, &current_reg);
    current_reg &= ~(7 << 2); //clear 3rd, 4th and 5th bits
    current_reg |= ((uint8_t)rate & 0x07) << 2;
    write8_mag(LSM303_REGISTER_MAG_CRA_REG_M, current_reg);
}

void get_Mag_Raw_Data(int16_t *x, int16_t *y, int16_t *z){
    uint8_t mag_out[6] = {0};
    read_bytes_mag(LSM303_REGISTER_MAG_OUT_X_H_M, mag_out, 6);
    *x = (int16_t)(mag_out[1] | (mag_out[0] << 8));
    *z = (int16_t)(mag_out[3] | (mag_out[2] << 8));
    *y = (int16_t)(mag_out[5] | (mag_out[4] << 8));
}

bool get_Mag_Data(float *x, float *y, float *z){
    uint8_t current_reg = 0;
    read8_mag(LSM303_REGISTER_MAG_SR_REG_Mg, &current_reg);
    current_reg &= 0x01; //clear all bits, only keep DRDY bit
    if(current_reg){ //if DRDY bit is set, read the data
        int16_t x_raw, y_raw, z_raw;
        x_raw = y_raw = z_raw = 0;
        get_Mag_Raw_Data(&x_raw, &y_raw, &z_raw);

        *x = (float)x_raw / _lsm303Mag_Gauss_LSB_XY;
        *y = (float)y_raw / _lsm303Mag_Gauss_LSB_XY;
        *z = (float)x_raw / _lsm303Mag_Gauss_LSB_Z;
        
        return true;
    }
    else{
        return false;
    }
}

bool LSM303_init(i2c_master_dev_handle_t *input_i2c_MagDev, i2c_master_dev_handle_t *input_i2c_AccelDev){
    if(input_i2c_AccelDev == NULL || input_i2c_MagDev == NULL){
        return false;
    }

    i2c_AccelDev_p = input_i2c_AccelDev;
    i2c_MagDev_p = input_i2c_MagDev;

    //Init Accel
    uint8_t data[16] = {0};
    data[0] = 0x57;

    write8_accel(LSM303_REGISTER_ACCEL_CTRL_REG1_A, data[0]); //Enable the accelerometer (100Hz)
    memset(data, 0, sizeof(data));

    // LSM303DLHC has no WHOAMI register so read CTRL_REG1_A back to check if we are connected or not
    read8_accel(LSM303_REGISTER_ACCEL_CTRL_REG1_A, &data[0]);
    if(data[0] != 0x57){
        printf("Accel failed to initialized \r\n");
        return false;
    }
    memset(data, 0, sizeof(data));

    //Init Mag
    // LSM303DLHC has no WHOAMI register but it has IRx_REG_M that should be constant
    read_bytes_mag(LSM303_REGISTER_MAG_IRA_REG_M, data, 3);
    if(data[0] != 0b01001000 || data[1] != 0b00110100 || data[2] != 0b00110011){
        printf("Magnetometer failed to initialized \r\n");
        return false;
    }
    memset(data, 0, sizeof(data));

    data[0] = 0x00;
    write8_mag(LSM303_REGISTER_MAG_MR_REG_M, data[0]); //Enable the magnetometer
    memset(data, 0, sizeof(data));

    setMagGain(LSM303_MAGGAIN_1_3);
    setMagRate(LSM303_MAGRATE_30);
    return true;
}

