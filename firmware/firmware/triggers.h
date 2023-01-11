/**
 * @file triggers.h
 * @author Roman Kiselev (roman.kiselev@stjude.org)
 * @brief Functions to read/write IO ports and generate trigger signals
 * @version 0.3
 * @date 2023-01-10
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include "sys_globals.h"

void init_IO();

void lasers_on();
void lasers_off();
uint8_t next_laser();