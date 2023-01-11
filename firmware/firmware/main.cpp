/*
 * Synchronization device for laser and camera control at pTIRF microscope
 *
 * @file main.cpp
 * @author Roman Kiselev (roman.kiselev@stjude.org)
 * @brief Synchronization device for laser and camera control at pTIRF microscope - main file
 * @version 0.3
 * @date 2022-01-10
 *
 * @copyright Copyright (c) 2023
*/ 

#include "sys_globals.h"
#include "triggers.h"
#include "timers.h"

uint32_t t0; // current system time
uint32_t interframe_time_us;
uint32_t n_frames = 4;
uint32_t n_acquired_frames; // Total number of acquired frames (pulses to camera)

void setup()
{
    init_IO();
	init_timer1();
	
	Serial.begin(9600);

	t0 = micros();
	interframe_time_us = 6120;
}

void loop()
{
	// if acquisition is running
	if (micros() - t0 > interframe_time_us)
	{
		// getting system time takes about 8 us
		t0 = micros() + 8;
		
		// start timer 1 over
		TCNT1 = ICR1 - 1;
		TCCR1B = bit(WGM12) | bit(WGM13) | TCCR1B_prescaler_bits;
	}
}

