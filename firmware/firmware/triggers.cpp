#include "triggers.h"


// Configure output ports (data direction registers and default values)
void init_IO()
{
	bitClear(FLUIDIC_PORT, FLUIDIC_PIN);
	bitSet(FLUIDIC_DDR, FLUIDIC_PIN);

	bitClear(CAMERA_PORT, CAMERA_PIN);
	bitSet(CAMERA_DDR, CAMERA_PIN);

	SHUTTERS_DDR |= SHUTTERS_MASK;
	SHUTTERS_PORT &= ~SHUTTERS_MASK;
}


void write_shutters(uint8_t value)
{
	SHUTTERS_PORT = (SHUTTERS_PORT & ~SHUTTERS_MASK) | value;
}