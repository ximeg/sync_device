/**
 * @file utils.h
 * @author Roman Kiselev (roman.kiselev@stjude.org)
 * @brief Miscellaneous untility functions (e.g. bit counting)
 * @version 0.3
 * @date 2022-08-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef UTILS_H
#define UTILS_H

#include "sys_globals.h"

// Bit manipulation
uint8_t count_bits(uint8_t v);
uint8_t decode_shutter_bits(uint8_t rx_bits);

#endif // UTILS_H