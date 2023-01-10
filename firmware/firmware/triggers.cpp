#include "triggers.h"


// Configure output ports (data direction registers and default values)
void setup_IO_ports()
{
	FLUIDIC_DDR |= bit(FLUIDIC_PIN);
	FLUIDIC_PORT &= ~bit(FLUIDIC_PIN);

	CAMERA_DDR |= bit(CAMERA_PIN);
	CAMERA_PORT &= ~bit(CAMERA_PIN);

	SHUTTERS_DDR |= SHUTTERS_MASK;
	SHUTTERS_PORT &= ~SHUTTERS_MASK;
}

void camera_pin_up()
{
	CAMERA_PORT |= bit(CAMERA_PIN);
}

void camera_pin_down()
{
	CAMERA_PORT &= ~bit(CAMERA_PIN);
}

void fluidic_pin_up()
{
	FLUIDIC_PORT |= bit(FLUIDIC_PIN);
}

void fluidic_pin_down()
{
	FLUIDIC_PORT &= ~bit(FLUIDIC_PIN);
}

void write_shutters(uint8_t value)
{
	SHUTTERS_PORT = (SHUTTERS_PORT & ~SHUTTERS_MASK) | value;
}