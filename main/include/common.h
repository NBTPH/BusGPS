/*
 * Common include in all components and programs
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

#define DEBUG_PRINT 0
#define I2C_TIMEOUT_MS 20 

int64_t millis();
int64_t micros();
void delay(uint32_t ms);
int debug_printf(const char *fmt,...);

#endif