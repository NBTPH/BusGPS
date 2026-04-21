#include <LSM303.h>

i2c_master_dev_handle_t i2c_MagDev;
i2c_master_dev_handle_t i2c_AccelDev;
lsm303MagGain magGain;

float _lsm303Accel_MG_LSB     = 0.001F;   // 1, 2, 4 or 12 mg per lsb
float _lsm303Mag_Gauss_LSB_XY = 1100.0F;  // Varies with gain
float _lsm303Mag_Gauss_LSB_Z  = 980.0F;   // Varies with gain

void write8_mag(const uint8_t reg_addr, const uint8_t data){
    uint8_t write_buffer[2] = {reg_addr, data};
    ESP_ERROR_CHECK(i2c_master_transmit(i2c_MagDev, write_buffer, 2, 250));
}
void read8_mag(const uint8_t reg_addr, const uint8_t *data){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_MagDev, &reg_addr, 1, data, 1, 250));
}

void write8_accel(const uint8_t reg_addr, const uint8_t data){
    uint8_t write_buffer[2] = {reg_addr, data};
    ESP_ERROR_CHECK(i2c_master_transmit(i2c_AccelDev, write_buffer, 2, 250));
}
void read8_accel(const uint8_t reg_addr, const uint8_t *data){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_AccelDev, &reg_addr, 1, data, 1, 250));
}

void write_bytes_accel(const uint8_t reg_addr, const uint8_t *data, size_t length){
    uint8_t *write_buffer = malloc(sizeof(uint8_t) * (length + 1));
    write_buffer[0] = reg_addr;
    memcpy(&write_buffer[1], data, length);
    ESP_ERROR_CHECK(i2c_master_transmit(i2c_MagDev, write_buffer, length + 1, 250));
    free(write_buffer);
}
void read_bytes_accel(const uint8_t reg_addr, const uint8_t *data, size_t length){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_MagDev, &reg_addr, 1, data, length, 250));
}

void write_bytes_mag(const uint8_t reg_addr, const uint8_t *data, size_t length){
    uint8_t *write_buffer = malloc(sizeof(uint8_t) * (length + 1));
    write_buffer[0] = reg_addr;
    memcpy(&write_buffer[1], data, length);
    ESP_ERROR_CHECK(i2c_master_transmit(i2c_AccelDev, write_buffer, length + 1, 250));
    free(write_buffer);
}
void read_bytes_mag(const uint8_t reg_addr, const uint8_t *data, size_t length){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_AccelDev, &reg_addr, 1, data, length, 250));
}

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
    uint8_t reg_m = ((uint8_t)rate & 0x07) << 2;
    write8_mag(LSM303_REGISTER_MAG_CRA_REG_M, reg_m);
}

bool LSM303_init(i2c_master_dev_handle_t i2c_dev){
    //Init Accel
    uint8_t data[16] = {0};
    data[0] = 0x57;

    write8_accel(LSM303_REGISTER_ACCEL_CTRL_REG1_A, data); //Enable the accelerometer (100Hz)
    memset(data, 0, sizeof(data));

    // LSM303DLHC has no WHOAMI register so read CTRL_REG1_A back to check if we are connected or not
    read8_accel(LSM303_REGISTER_ACCEL_CTRL_REG1_A, &data);
    if(data != 0x57){
        return false;
    }
    memset(data, 0, sizeof(data));

    //Init Mag
    // LSM303DLHC has no WHOAMI register but it has IRx_REG_M that should be constant
    read_bytes_mag(LSM303_REGISTER_MAG_IRA_REG_M, &data, 3);
    if(data[0] != 0b01001000 || data[1] != 0b00110100 || data[2] != 0b00110011){
        return false;
    }
    memset(data, 0, sizeof(data));

    data[0] = 0x00;
    write8_mag(LSM303_REGISTER_MAG_MR_REG_M, data); //Enable the magnetometer
    memset(data, 0, sizeof(data));

    setMagGain(LSM303_MAGGAIN_1_3);
    return true;
}

void getAcceldata(lsm303AccelData *accel){
    uint8_t accel_out[6] = {0};
    read_bytes_accel(LSM303_REGISTER_ACCEL_OUT_X_L_A, accel_out, 6);

    accel->x =  (float)(raw.x >> shift) * lsb * SENSORS_GRAVITY_STANDARD;
}