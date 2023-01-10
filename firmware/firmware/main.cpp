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

#include <avr/io.h>
#include "sys_globals.h"
#include "triggers.h"

int main(void)
{
    setup_IO_ports();
    while (1) 
    {
		camera_pin_up();
		fluidic_pin_up();
		write_shutters(0x0f);
		write_shutters(0x03);
		write_shutters(0x07);
		write_shutters(0x0a);
		camera_pin_down();
		fluidic_pin_down();
		
    }
}

