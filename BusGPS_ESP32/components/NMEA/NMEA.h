#ifndef __L76G_H__
#define __L76G_H__

#include <common.h>
#include <stdlib.h>

typedef struct{
    uint8_t Hours;
    uint8_t Minutes;
    float Seconds;
}UTC_t;

typedef struct{
    uint8_t Day;
    uint8_t Month;
    uint8_t Year;
}Date_t;

typedef struct{
    UTC_t UTC;
    char Status;
    float Lat;
    char NS;
    float Lon;
    char EW;
    float SOG; //Knots
    float COG; //Degree
    Date_t DATE;
    // <MagVar> - - - Magnetic variation. Not supported. 
    // <MagVarDir> - - - Direction of magnetic variation.
    char ModeInd;
    char NavStatus;
    uint8_t CSM; //8 bit checksum
}RMC_MSG_t;

typedef struct{
    UTC_t UTC;
    float Lat;
    char NS;
    float Lon;
    char EW;
    uint8_t Quality;
    uint8_t NumSatUsed;
    float HDOP;
    float Alt;
    float Sep;
    // <DiffAge> Not supported.
    // <DiffStation> Not supported.
    uint8_t CSM; //8 bit checksum
}GGA_MSG_t;

typedef struct{
    uint16_t CommandID;
    uint8_t Result;
}ACK_MSG_t;

/* Checksum is the 8-bit exclusive OR of all characters in the sentence, including the ‘,’ 
field delimiter, between but not including the ‘$’ and the ‘*’ delimiters. */
uint8_t Ql_Check_XOR(const char *pData, unsigned int Length);

void Print_RMC_Data(const RMC_MSG_t *msg);
bool Parse_RMC_MSG(const char *const p_start, unsigned int Length, RMC_MSG_t *msg_data);

void Print_GGA_Data(const GGA_MSG_t *msg);
bool Parse_GGA_MSG(const char *const p_start, unsigned int Length, GGA_MSG_t *msg_data);

bool Parse_ACK_MSG(const char *const p_start, unsigned int Length, ACK_MSG_t *msg_data);
void Set_Fix_Rate_MSG(uint16_t ms, char *out_tx_buffer);
#endif