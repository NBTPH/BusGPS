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
    float bias_corrected[3] = {0};
    bias_corrected[0] = (((float)x_raw) / _hmc5883_Gauss_LSB_XYZ) - hard_iron_bias[0];
    bias_corrected[1] = (((float)y_raw) / _hmc5883_Gauss_LSB_XYZ) - hard_iron_bias[1];
    bias_corrected[2] = (((float)z_raw) / _hmc5883_Gauss_LSB_XYZ) - hard_iron_bias[2];

    float calibrated_data[3] = {0};
    _mulvec((float*)soft_iron_matrix, bias_corrected, calibrated_data, 3, 3);
    
    *x = calibrated_data[0];
    *y = calibrated_data[1];
    *z = calibrated_data[2];
}

bool HMC5883_is_Calibrating = false;
void HMC5883_Calibration(uint32_t samples_num){
    if(HMC5883_is_Calibrating){
        return;
    }
    HMC5883_is_Calibrating = true;
    delay(5);
    printf("=========== HMC5883 Calibrations begin ===========\r\n");

    float scatter_matrix[10][10]; //Declare the pointer with the internal dimension (10 one sample entry)
    float previous_sample[3] = {0, 0, 1};
    memset((float*)scatter_matrix, 0, sizeof(scatter_matrix)); //initalize all the maxtrix
    uint32_t num = 0;
    while(num < samples_num){
        while(!HMC5883_DataReady()); //wait until data ready
        float data_read[3];
        getMagData(&data_read[0], &data_read[1], &data_read[2]); //
        
        float angle = 0;
        angle_between(data_read, previous_sample, &angle, 3);
        if(angle > 10){//take in new data only when new data vector is more than 10 degree from previous data vector

            //vi = x^2, y^2, z^2, xy, yz, xz, x, y, z, 1
            float entry[10];
            entry[0] = data_read[0] * data_read[0];
            entry[1] = data_read[1] * data_read[1];
            entry[2] = data_read[2] * data_read[2];

            entry[3] = data_read[0] * data_read[1];
            entry[4] = data_read[1] * data_read[2];
            entry[5] = data_read[0] * data_read[2];

            entry[6] = data_read[0];
            entry[7] = data_read[1];
            entry[8] = data_read[2];

            entry[9] = 1;

            //Only calculate the upper triangle and mirror it later
            for (int i = 0; i < 10; i++) {
                for (int j = i; j < 10; j++) {
                    scatter_matrix[i][j] += entry[i] * entry[j];
                }
            }
            
            memcpy(previous_sample, data_read, sizeof(previous_sample));
            if(((num + 1) % 5) == 0){
                printf("Sample %lu/%lu: x=%.4f y=%.4f z=%.4f\r\n", (unsigned long)num + 1, (unsigned long)samples_num, data_read[0], data_read[1], data_read[2]);
            }
            num++;
        }
        delay(10);
    }

    //mirror the lower triangle
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < i; j++){
            scatter_matrix[i][j] = scatter_matrix[j][i];
        }
    }

    debug_printf("Scatter matrix:\n");
    print_matrix((float*)scatter_matrix, 10, 10);

    debug_printf("Running Jacobi eigensystem...\n");
    float eigenvalues[10] = {0};
    float eigenvectors[10][10] = {0};
    if(!jacobi_eigensystem((float*)scatter_matrix, eigenvalues, (float*)eigenvectors, 10)){
        return;
    }

    debug_printf("Eigenvalues: ");
    print_vector(eigenvalues, 10);
    debug_printf("Eigenvectors:\n");
    print_matrix((float*)eigenvectors, 10, 10);

    float smallest_nonneg_eigenvalue = -1;
    int8_t smallest_idx = -1;

    for(uint8_t i = 0; i < 10; i++){
        if(smallest_nonneg_eigenvalue < 0 && eigenvalues[i] >= 0){ //get the first non neg eigen value
            smallest_nonneg_eigenvalue = eigenvalues[i];
            smallest_idx = i;
        }
        else if(smallest_nonneg_eigenvalue >= 0 && smallest_idx >= 0){ //if found the first nonneg eigen value, find the smallest nonneg eigenvalue
            if(eigenvalues[i] < smallest_nonneg_eigenvalue && eigenvalues[i] >= 0){
                smallest_nonneg_eigenvalue = eigenvalues[i];
                smallest_idx = i;
            }
        }
    }

    debug_printf("Smallest non-negative eigenvalue: %10.6f at index %d\n", smallest_nonneg_eigenvalue, smallest_idx);
    debug_printf("Corresponding eigenvector: ");
    print_vector(&eigenvectors[smallest_idx][0], 10);

    //The eigenvector corresponds to the smallest eigenvalue is the 
    //Only copy the upper triangle and mirror it later
    float symmetric_matrix[3][3] = {0};
    symmetric_matrix[0][0] = eigenvectors[smallest_idx][0]; 
    symmetric_matrix[0][1] = eigenvectors[smallest_idx][3] / 2.f;
    symmetric_matrix[0][2] = eigenvectors[smallest_idx][5] / 2.f;
    symmetric_matrix[1][1] = eigenvectors[smallest_idx][1];
    symmetric_matrix[1][2] = eigenvectors[smallest_idx][4] / 2.f;
    symmetric_matrix[2][2] = eigenvectors[smallest_idx][2];

    //mirror the lower triangle
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < i; j++){
            symmetric_matrix[i][j] = symmetric_matrix[j][i];
        }
    }

    debug_printf("Symmetric matrix M:\n");
    print_matrix((float*)symmetric_matrix, 3, 3);

    //calculate the hard iron bias vector b = (-1/2) * symmetric_matrix^-1 * [a7, a8, a9] (put this in a code block so we don't waist too much stack memory
    {
        float inverse_symmetric_matrix[3][3] = {0};
        invert((float*)symmetric_matrix, (float*)inverse_symmetric_matrix, 3);

        debug_printf("Invert of M:\n");
        print_matrix((float*)inverse_symmetric_matrix, 3, 3);

        float temp_hard_iron_bias[3] = {0};
        _mulvec((float*)inverse_symmetric_matrix, &eigenvectors[smallest_idx][6], temp_hard_iron_bias, 3, 3);

        for(uint8_t i = 0; i < 3; i++){
            hard_iron_bias[i] = temp_hard_iron_bias[i] * -(0.5); //why is some paper just multiply with -1 while some multiply with -1/2
        }

        debug_printf("Hard iron bias (Gauss): x=%.6f y=%.6f z=%.6f\n", hard_iron_bias[0], hard_iron_bias[1], hard_iron_bias[2]);
    }


    //calculate the scaling factor k = R^2 / (b^TMb) - a10
    float k = 0;
    {
        float R = 41841.5f / 100000.0f; //the total magnetic intensity is in nano tesla and magnetometer output gauss so we convert to gauss
        float Mb[3] = {0};
        _mulvec((float*)symmetric_matrix, hard_iron_bias, Mb, 3, 3);

        float bTMb = 0;
        for(uint8_t i = 0; i < 3; i++){
            bTMb += Mb[i] * hard_iron_bias[i];
        }

        k = (R * R) / (bTMb - eigenvectors[smallest_idx][9]);

        debug_printf("R=%.6f Gauss, bTMb=%.6f, a10=%.6f\n", R, bTMb, eigenvectors[smallest_idx][9]);
        debug_printf("Scaling factor k=%.6f\n", k);

        if(k <= 0){ //invalid scaling factor
            memset(hard_iron_bias, 0, sizeof(hard_iron_bias));
            return;
        }
    }

    float scaled_symmetric_matrix[3][3] = {0};
    for(uint8_t i = 0; i < 3; i++){
        for(uint8_t j = 0; j < 3; j++){
            scaled_symmetric_matrix[i][j] = symmetric_matrix[i][j] * k;
        }
    }

    debug_printf("Scaled symmetric matrix M:\n");
    print_matrix((float*)scaled_symmetric_matrix, 3, 3);

    float s_eigenvalues[3] = {0};
    float s_eigenvectors[3][3] = {0};
    if(!jacobi_eigensystem((float*)scaled_symmetric_matrix, s_eigenvalues, (float*)s_eigenvectors, 3)){
        return;
    }

    debug_printf("Scaled eigenvalues: ");
    print_vector((float*)s_eigenvalues, 3);

    
    debug_printf("Scaled eigenvectors:\n");
    print_matrix((float*)s_eigenvectors, 3, 3);


    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            soft_iron_matrix[i][j] = 0;
            for (int m = 0; m < 3; m++) {
                // V_{i,m} is M_eigenvectors[m*3 + i]
                // sqrt(D_{m}) is sqrt(M_eigenvalues[m])
                // V^T_{m,j} is M_eigenvectors[m*3 + j]
                soft_iron_matrix[i][j] += s_eigenvectors[m][i] * sqrtf(fabsf(s_eigenvalues[m])) * s_eigenvectors[m][j]; //C^-1 = V^T sqrt(D) V
            }
        }
    }

    debug_printf("Soft iron matrix W:\n");
    print_matrix((float*)soft_iron_matrix, 3, 3);

    HMC5883_is_Calibrating = false;
    printf("=========== HMC5883 Calibrations Complete ===========\r\n\n");
    printf("Hard iron bias (Gauss): x=%.6f y=%.6f z=%.6f\n\n", hard_iron_bias[0], hard_iron_bias[1], hard_iron_bias[2]);
    printf("Soft iron matrix W:\n");
    for(int i = 0; i < 3; i++){
        printf("[ ");
        for(int j = 0; j < 3; j++) printf("%10.6f ", soft_iron_matrix[i][j]);
        printf("]\n");
    }
    printf("\n");
}

bool HMC5883_Init(i2c_master_dev_handle_t *input_hmc5883_dev){
    _addeye((float*)soft_iron_matrix, 3); //initalize soft_iron_matrix as identity matrix

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