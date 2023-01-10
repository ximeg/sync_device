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


int main(void)
{
    setup_IO_ports();
	setup_timer1();
	sei();
    while (1) 
    {
		;
    }
}

