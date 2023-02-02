/**
 * @file triggers.h
 * @author Roman Kiselev (roman.kiselev@stjude.org)
 * @brief Functions to read/write IO ports and generate trigger signals
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include "sys_globals.h"

// Initialize all I/O ports
void init_IO();

// Write data to I/O port that controls laser shutters
void set_lasers(uint8_t laser);

// Close all laser shutters
void lasers_off();

// Return bit that represents next ALEX laser among all enabled lasers
// Wraps around after the last laser and starts over with the first one
uint8_t next_laser();

// Returns bit representing first laser for the ALEX mode
uint8_t get_first_laser();

// Changes currently active laser to first laser if ALEX mode is on or
// activates all enabled lasers if ALEX mode is off.
void reset_lasers();

