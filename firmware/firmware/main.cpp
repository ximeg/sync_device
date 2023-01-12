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
#include "uart.h"

SystemSettings sys = {
	IDLE,    // STATUS   status;
	5UL,  // uint32_t shutter_delay_us;
	25UL, // uint32_t cam_readout_us;
	10UL,  // uint32_t exp_time_us;
	200UL, // uint32_t acq_period_us; // at least the sum of all three above
	0,       // uint32_t n_frames;
	0,       // uint32_t n_acquired_frames;
	bit(CY2_PIN), // uint8_t current_laser;
	SHUTTERS_MASK
	};

int main(void)
{
    init_IO();
	init_UART();
	sei();

	start_acq();

	UART_tx("Big hello to this world!");

	char buf;
    while (1) 
    {
		buf = UART_rx();
		UART_tx(buf + 1);
    }
}

