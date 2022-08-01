#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <Arduino.h>
#include "sys_globals.h"

// Bit manipulation
uint8_t count_bits(uint8_t v);
uint8_t decode_shutter_bits(uint8_t rx_bits);

#endif // UTILS_H