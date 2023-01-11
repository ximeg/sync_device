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

/**********************
SYSTEM STATUS VARIABLES
**********************/
enum STATUS
{
	IDLE = 0,            // Doing nothing, waiting for commands
	FRAME,               // Data acquisition (triggering of lasers and camera using TC1)
	INTERFRAME_DELAY,    // Pause between frames (controlled by TC3)
};

typedef struct SystemSettings
{
	STATUS   status;
	uint32_t shutter_delay_us;  // Laser shutter delay, 1000us by default
	uint32_t cam_readout_us;    // Camera readout time, 12 ms by default. Depends on the ROI size
	uint32_t exp_time_us;       // Exposure time = duration of laser shutter being open
	uint32_t acq_period_us;     // Time period between subsequent frames or bursts of frames (ALEX)
	// n_frames_per_burst? // n_bursts??
	uint32_t n_frames;          // 0 means unlimited
	uint32_t n_acquired_frames; // Total number of acquired frames (pulses to camera)
	uint8_t  active_lasers;     // Bit pattern to control laser shutters. Bit 0: Cy2, bit 3: Cy7
} SystemSettings;

extern SystemSettings sys;
