/*
 * Common include in all components and programs
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

#define I2C_TIMEOUT_MS 20 

int64_t millis();
int64_t micros();
void delay(uint32_t ms);

#endif