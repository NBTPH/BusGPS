#include <HMC5883_U.h>
#include <HMC5883.h>

i2c_master_dev_handle_t *hmc5883_dev_p = NULL;

float soft_iron_matrix[3][3] = {0};
float hard_iron_bias[3] = {0};

static void write8(const uint8_t reg_addr, const uint8_t data){
    uint8_t write_buff[2] = {reg_addr, data};
    ESP_ERROR_CHECK(i2c_master_transmit(*hmc5883_dev_p, write_buff, 2, I2C_TIMEOUT_MS));
}
static void read8(const uint8_t reg_addr, uint8_t *data){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*hmc5883_dev_p, &reg_addr, 1, data, 1, I2C_TIMEOUT_MS));
}

static void write_bytes(const uint8_t reg_addr, const uint8_t *data, size_t length){
    uint8_t *write_buff = malloc(sizeof(uint8_t) * (length + 1));
    write_buff[0] = reg_addr;
    memcpy(&write_buff[1], data, length);
    ESP_ERROR_CHECK(i2c_master_transmit(*hmc5883_dev_p, write_buff, length + 1, I2C_TIMEOUT_MS));
    free(write_buff);
}
static void read_bytes(const uint8_t reg_addr, uint8_t *data, size_t length){
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*hmc5883_dev_p, &reg_addr, 1, data, length, I2C_TIMEOUT_MS));
}

hmc5883SamplesAverage_t getSamplesAverage(void){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &reg);
    reg &= (0b11 << 5); //extract 7th and 6th bits
    return (hmc5883SamplesAverage_t)(reg >> 5); //shifts back
}
bool setSamplesAverage(hmc5883SamplesAverage_t samavr){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &reg);
    reg &= ~(0b11 << 5); //clear 7th and 6th bits
    reg |= (samavr << 5); 
    write8(HMC5883_REGISTER_MAG_CRA_REG_M, reg);

    uint8_t check = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &check);
    return ((check >> 5) & 0b11) == (uint8_t)samavr;
}

hmc5883MagRate_t getMagRate(void){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &reg);
    reg &= (0b111 << 2); // Extract 5th, 4th, and 3rd bits
    return (hmc5883MagRate_t)(reg >> 2);
}
bool setMagRate(hmc5883MagRate_t rate){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &reg);
    reg &= ~(0b111 << 2); // Clear 5th, 4th, and 3rd bits
    reg |= (rate << 2); 
    write8(HMC5883_REGISTER_MAG_CRA_REG_M, reg);

    uint8_t check = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &check);
    return ((check >> 2) & 0b111) == (uint8_t)rate;
}

hmc5883MeasureMode_t getMeasureMode(void){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &reg);
    reg &= (0b11 << 0); // Extract 2nd and 1st bits
    return (hmc5883MeasureMode_t)reg;
}

bool setMeasureMode(hmc5883MeasureMode_t mode){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &reg);
    reg &= ~(0b11 << 0); // Clear 2nd and 1st bits
    reg |= mode; 
    write8(HMC5883_REGISTER_MAG_CRA_REG_M, reg);

    uint8_t check = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &check);
    return (check & 0b11) == (uint8_t)mode;
}

hmc5883MagGain_t getMagGain(void){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_CRB_REG_M, &reg);
    reg &= (0b111 << 5); // Extract 8th, 7th, and 6th bits
    return (hmc5883MagGain_t)(reg >> 5);
}

bool setMagGain(hmc5883MagGain_t gain){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_CRB_REG_M, &reg);
    reg &= ~(0b111 << 5); // Clear 8th, 7th, and 6th bits
    reg |= (gain << 5); 
    write8(HMC5883_REGISTER_MAG_CRB_REG_M, reg);

    uint8_t check = 0;
    read8(HMC5883_REGISTER_MAG_CRB_REG_M, &check);
    return ((check >> 5) & 0b111) == (uint8_t)gain;
}

hmc5883OperateMode_t getOperateMode(void){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_MR_REG_M, &reg);
    reg &= (0b11 << 0); // Extract 2nd and 1st bits
    return (hmc5883OperateMode_t)reg;
}

bool setOperateMode(hmc5883OperateMode_t opMode){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_MR_REG_M, &reg);
    reg &= ~(0b11 << 0); // Clear 2nd and 1st bits
    reg |= opMode; 
    write8(HMC5883_REGISTER_MAG_MR_REG_M, reg);

    uint8_t check = 0;
    read8(HMC5883_REGISTER_MAG_MR_REG_M, &check);
    return (check & 0b11) == (uint8_t)opMode;
}

void getMagRawData(int16_t *x, int16_t *y, int16_t *z){
    uint8_t reg[6] = {0};
    read_bytes(HMC5883_REGISTER_MAG_OUT_X_H_M, reg, 6);
    *x = (int16_t)((reg[0] << 8) | reg[1]);
    *z = (int16_t)((reg[2] << 8) | reg[3]);
    *y = (int16_t)((reg[4] << 8) | reg[5]);
}

void enableMag(void){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_CRA_REG_M, &reg);
    reg &= ~(1 << 7); // Clear 7
    write8(HMC5883_REGISTER_MAG_CRA_REG_M, reg); //write back

    reg = 0;
    read8(HMC5883_REGISTER_MAG_MR_REG_M, &reg);
    reg &= ~(1 << 7); // Clear 7
    write8(HMC5883_REGISTER_MAG_MR_REG_M, reg); //write back
}

bool HMC5883_DataReady(void){
    uint8_t reg = 0;
    read8(HMC5883_REGISTER_MAG_SR_REG_M, &reg);
    reg &= 0x01;
    return reg;
}

void getMagData(float *x, float *y, float *z){
    //get LSB sensitivity
    hmc5883MagGain_t mag_gain = getMagGain();
    float _hmc5883_Gauss_LSB_XYZ = 1090;
    switch (mag_gain) {
        case HMC5883_MAGGAIN_0_8:
            _hmc5883_Gauss_LSB_XYZ = 1370;
            break;
        case HMC5883_MAGGAIN_1_3:
            _hmc5883_Gauss_LSB_XYZ = 1090;
            break;
        case HMC5883_MAGGAIN_1_9:
            _hmc5883_Gauss_LSB_XYZ = 820;
            break;
        case HMC5883_MAGGAIN_2_5:
            _hmc5883_Gauss_LSB_XYZ = 660;
            break;
        case HMC5883_MAGGAIN_4_0:
            _hmc5883_Gauss_LSB_XYZ = 440;
            break;
        case HMC5883_MAGGAIN_4_7:
            _hmc5883_Gauss_LSB_XYZ = 390;
            break;
        case HMC5883_MAGGAIN_5_6:
            _hmc5883_Gauss_LSB_XYZ = 330;
            break;
        case HMC5883_MAGGAIN_8_1:
            _hmc5883_Gauss_LSB_XYZ = 230;
            break;
        default:
            break;
    }

    //get raw data
    int16_t x_raw, y_raw ,z_raw;
    x_raw = y_raw = z_raw = 0;
    getMagRawData(&x_raw, &y_raw, &z_raw);
    //scale raw data
    *x = ((float)x_raw) / _hmc5883_Gauss_LSB_XYZ;
    *y = ((float)y_raw) / _hmc5883_Gauss_LSB_XYZ;
    *z = ((float)z_raw) / _hmc5883_Gauss_LSB_XYZ;
}

bool HMC5883_is_Calibrating = false;
void HMC5883_Calibration(uint32_t samples_num){
    if(!HMC5883_is_Calibrating){
        HMC5883_is_Calibrating = true;

        float scatter_matrix[10][10]; //Declare the pointer with the internal dimension (10 one sample entry)
        memset(scatter_matrix, 0, sizeof(scatter_matrix)); //initalize all the maxtrix
        for(uint32_t i = 0; i < samples_num; i++){
            while(!HMC5883_DataReady()); //wait until data ready
            float x = 0, y = 0, z = 0;
            getMagData(&x, &y, &z);

            //vi = x^2, y^2, z^2, xy, yz, xz, x, y, z, 1
            float entry[10];
            entry[i][0] = x * x;
            entry[i][1] = y * y;
            entry[i][2] = z * z;

            entry[i][3] = x * y;
            entry[i][4] = y * z;
            entry[i][5] = x * z;

            entry[i][6] = x;
            entry[i][7] = y;
            entry[i][8] = z;

            entry[i][9] = 1;

            //Only calculate the upper triangle and mirror it later
            for (int i = 0; i < 10; i++) {
                for (int j = i; j < 10; j++) {
                    scatter_matrix[i][j] += entry[i] * entry[j];
                }
            }

            delay(50);
        }

        //mirror the lower triangle
        for(int i = 0; i < 10; i++){
            for(int j = 0; j < i; j++){
                scatter_matrix[i][j] = scatter_matrix[j][i];
            }
        }

        float eigenvalues[10] = {0};
        float eigenvector[10][10] = {0};
        if(!jacobi_eigensystem(scatter_matrix, eigenvalues, eigenvector, 10)){
            return;
        }

        float smallest_nonneg_eigenvalue = -1;
        uint8_t smallest_nonneg_eigenvalue_idx = -1;

        for(uint8_t i = 1; i < 10; i++){
            if(smallest_nonneg_eigenvalue < 0 && eigenvalues[i] >= 0){ //get the first non neg eigen value
                smallest_nonneg_eigenvalue = eigenvalues[i];
                smallest_nonneg_eigenvalue_idx = i;
            }
            else if(smallest_nonneg_eigenvalue >= 0 && smallest_nonneg_eigenvalue_idx >= 0){
                if(eigenvalues[i] < smallest_nonneg_eigenvalue && eigenvalues[i] >= 0){
                    smallest_nonneg_eigenvalue = eigenvalues[i];
                    smallest_nonneg_eigenvalue_idx = i;
                }
            }
        }

        //Only copy the upper triangle and mirror it later
        float symmetric_matrix[3][3] = {0};
        symmetric_matrix[0][0] = eigenvector[smallest_nonneg_eigenvalue_idx][0]; 
        symmetric_matrix[0][1] = eigenvector[smallest_nonneg_eigenvalue_idx][3] / 2.f;
        symmetric_matrix[0][2] = eigenvector[smallest_nonneg_eigenvalue_idx][4] / 2.f;
        symmetric_matrix[1][1] = eigenvector[smallest_nonneg_eigenvalue_idx][1];
        symmetric_matrix[1][2] = eigenvector[smallest_nonneg_eigenvalue_idx][5] / 2.f;
        symmetric_matrix[2][2] = eigenvector[smallest_nonneg_eigenvalue_idx][2];

        //mirror the lower triangle
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < i; j++){
                symmetric_matrix[i][j] = symmetric_matrix[j][i];
            }
        }

        float inverse_symmetric_matrix[3][3] = {0};
        invert(symmetric_matrix, inverse_symmetric_matrix, 3);

        float temp_hard_iron_bias[3] = {0};
        _mulvec(inverse_symmetric_matrix, &eigenvector[smallest_nonneg_eigenvalue_idx][6], temp_hard_iron_bias, 3, 3);

        for(uint8_t i = 0; i < 3; i++){
            hard_iron_bias[i] = temp_hard_iron_bias[i] * -(0.5); //why is some paper just multiply with -1 while some multiply with -1/2
        }

        
    }
}

bool HMC5883_Init(i2c_master_dev_handle_t *input_hmc5883_dev){
    if(input_hmc5883_dev == NULL){
        printf("Error: MPU6050 I2C dev handler not provided\r\n");
        return false;
    }
    hmc5883_dev_p = input_hmc5883_dev;

    printf("Begin initalizing HMC5883\r\n");

    uint8_t reg[4] = {0};
    read_bytes(HMC5883_REGISTER_MAG_IRA_REG_M, reg, 3);
    if(reg[0] != 0b01001000 || reg[1] != 0b00110100 || reg[2] != 0b00110011){
        printf("Mag failed ID registers test\r\n");
        return false;
    }

    enableMag();

    if(!setOperateMode(HMC5883_OP_MODE_CONTINUOUS)) printf("Error: Failed to set Operation Mode\r\n");

    if(!setMeasureMode(HMC5883_MEASURE_MODE_NORMAL)) printf("Error: Failed to set Measurement Mode\r\n");

    if(!setMagGain(HMC5883_MAGGAIN_1_3)) printf("Error: Failed to set Magnetometer Gain\r\n");

    if(!setMagRate(HMC5883_MAGRATE_30)) printf("Error: Failed to set Magnetometer Rate\r\n");

    return true;
}