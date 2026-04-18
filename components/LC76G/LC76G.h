#ifndef __L76G_H__
#define __L76G_H__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
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
    uint8_t Quality; //Knots
    uint8_t NumSatUsed; //Degree
    float HDOP;
    float Alt;
    float Sep;
    // <DiffAge> Not supported.
    // <DiffStation> Not supported.
    uint8_t CSM; //8 bit checksum
}GGA_MSG_t;

/* Checksum is the 8-bit exclusive OR of all characters in the sentence, including the ‘,’ 
field delimiter, between but not including the ‘$’ and the ‘*’ delimiters. */
bool Parse_RMC_MSG(const char *const p_start, unsigned int Length, RMC_MSG_t *msg_data);
bool Parse_GGA_MSG(const char *const p_start, unsigned int Length, GGA_MSG_t *msg_data);

#endif