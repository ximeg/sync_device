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


void lasers_on()
{
	SHUTTERS_PORT |= sys.active_lasers & SHUTTERS_MASK;
}

void lasers_off()
{
	SHUTTERS_PORT &= ~SHUTTERS_MASK;
}

void next_laser()
{
	switch (sys.active_lasers)
	{
	case bit(CY2_PIN):
		sys.active_lasers = bit(CY3_PIN);
	break;
	case bit(CY3_PIN):
		sys.active_lasers = bit(CY5_PIN);
	break;
	case bit(CY5_PIN):
		sys.active_lasers = bit(CY7_PIN);
	break;
	case bit(CY7_PIN):
		sys.active_lasers = bit(CY2_PIN);
	break;
	}
}