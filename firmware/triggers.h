/**
 * @file triggers.h
 * @author Roman Kiselev (roman.kiselev@stjude.org)
 * @brief Functions to read/write IO ports to generate trigger signals
 * @version 0.3
 * @date 2022-08-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef TRIGGERS_H
#define TRIGGERS_H

#include "sys_globals.h"

void setup_IO_ports();

void camera_pin_up();
void camera_pin_down();

void fluidic_pin_up();
void fluidic_pin_down();

void write_shutters(uint8_t value);

#endif // TRIGGERS_H