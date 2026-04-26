#ifndef _HMC5883_U_H
#define _HMC5883_U_H

#include <common.h>
#include <stdlib.h>
#include <LinearAlgebra.h>
#include "driver/i2c_master.h"
#include "esp_check.h"

#define HMC5883_REGISTER_MAG_CRA_REG_M 0x00
#define HMC5883_REGISTER_MAG_CRB_REG_M 0x01
#define HMC5883_REGISTER_MAG_MR_REG_M 0x02
#define HMC5883_REGISTER_MAG_OUT_X_H_M 0x03
#define HMC5883_REGISTER_MAG_OUT_X_L_M 0x04
#define HMC5883_REGISTER_MAG_OUT_Z_H_M 0x05
#define HMC5883_REGISTER_MAG_OUT_Z_L_M 0x06
#define HMC5883_REGISTER_MAG_OUT_Y_H_M 0x07
#define HMC5883_REGISTER_MAG_OUT_Y_L_M 0x08
#define HMC5883_REGISTER_MAG_SR_REG_M 0x09
#define HMC5883_REGISTER_MAG_IRA_REG_M 0x0A
#define HMC5883_REGISTER_MAG_IRB_REG_M 0x0B
#define HMC5883_REGISTER_MAG_IRC_REG_M 0x0C

typedef enum {
	HMC5883_SAMAVR_1 = 0b00, //1 sample
	HMC5883_SAMAVR_2 = 0b01, //2 samples
	HMC5883_SAMAVR_4 = 0b10, //4 samples
	HMC5883_SAMAVR_8 = 0b11 //8 samples
} hmc5883SamplesAverage_t;  //bit 7th and 6th bits of HMC5883_REGISTER_MAG_CRA_REG_M

typedef enum {
	HMC5883_MAGRATE_0_7 = 0b0000, //0.75 Hz
    HMC5883_MAGRATE_1_5 = 0b0001, //1.5 Hz
    HMC5883_MAGRATE_3 = 0b0010, //3 Hz
    HMC5883_MAGRATE_7_5 = 0b0011, //7.5 Hz
    HMC5883_MAGRATE_15 = 0b0100, //15 Hz
    HMC5883_MAGRATE_30 = 0b0101, //30 Hz
    HMC5883_MAGRATE_75 = 0b0110 //75 Hz
} hmc5883MagRate_t; //bit 5th, 4th and 3rd bits of HMC5883_REGISTER_MAG_CRA_REG_M

typedef enum {
	HMC5883_MEASURE_MODE_NORMAL = 0b00, //1 sample
	HMC5883_MEASURE_MODE_POS = 0b01, //2 samples
	HMC5883_MEASURE_MODE_NEG = 0b10 //4 samples
} hmc5883MeasureMode_t; //bit 2nd, 1st bits of HMC5883_REGISTER_MAG_CRA_REG_M

typedef enum {
    HMC5883_MAGGAIN_0_8 = 0b0000, // +/- 0.88
	HMC5883_MAGGAIN_1_3 = 0b0001, // +/- 1.3
	HMC5883_MAGGAIN_1_9 = 0b0010, // +/- 1.9
	HMC5883_MAGGAIN_2_5 = 0b0011, // +/- 2.5
	HMC5883_MAGGAIN_4_0 = 0b0100, // +/- 4.0
	HMC5883_MAGGAIN_4_7 = 0b0101, // +/- 4.7
	HMC5883_MAGGAIN_5_6 = 0b0110, // +/- 5.6
	HMC5883_MAGGAIN_8_1 = 0b0111  // +/- 8.1
} hmc5883MagGain_t; //bit 8th, 7th and 6th of HMC5883_REGISTER_MAG_CRB_REG_M

typedef enum {
	HMC5883_OP_MODE_CONTINUOUS = 0b00, //1 sample
	HMC5883_OP_MODE_SINGLE = 0b01, //2 samples
	HMC5883_OP_MODE_IDLE = 0b10 //4 samples
} hmc5883OperateMode_t; //bit 2nd and 1st of HMC5883_REGISTER_MAG_MR_REG_M

#endif