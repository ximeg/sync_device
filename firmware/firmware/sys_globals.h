/**
 * @file sys_globals.h
 * @author Roman Kiselev (roman.kiselev@stjude.org)
 * @brief Global system definitions: pin out and settings.
 * @version 0.3
 * @date 2022-01-10
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
//#include "sys_time.h"


/*********************************
HELPFUL BIT MANIPULATION FUNCTIONS
*********************************/
#define bit(b)                         (1UL << (b))
#define bitRead(value, bit)            (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)             ((value) |= (1UL << (bit)))
#define bitClear(value, bit)           ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit)          ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

/****************************
PINOUT AND WIRING DEFINITIONS
****************************/
// Laser shutters
const uint8_t CY2_PIN = PINF0; // aka Arduino pin A0
const uint8_t CY3_PIN = PINF1; // aka Arduino pin A1
const uint8_t CY5_PIN = PINF2; // aka Arduino pin A2
const uint8_t CY7_PIN = PINF3; // aka Arduino pin A3
const uint8_t SHUTTERS_MASK = bit(CY2_PIN) | bit(CY3_PIN) | bit(CY5_PIN) | bit(CY7_PIN);
#define SHUTTERS_PORT PORTF
#define SHUTTERS_DDR DDRF

// Fluidic system trigger
const uint8_t FLUIDIC_PIN = PINE4; // aka Arduino pin 2
#define FLUIDIC_PORT PORTE
#define FLUIDIC_DDR DDRE

// Camera trigger
const uint8_t CAMERA_PIN = PINB7; // aka Arduino pin 13
#define CAMERA_PORT PORTB
#define CAMERA_DDR DDRB
