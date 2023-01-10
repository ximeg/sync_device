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

void setup_IO_ports();

void camera_pin_up();
void camera_pin_down();

void fluidic_pin_up();
void fluidic_pin_down();

void write_shutters(uint8_t value);

