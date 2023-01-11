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
	SHUTTERS_PORT |= sys.current_laser & SHUTTERS_MASK;
}

void lasers_off()
{
	SHUTTERS_PORT &= ~SHUTTERS_MASK;
}

uint8_t next_laser()
{
	switch (sys.current_laser)
	{
	case bit(CY2_PIN):
		return bit(CY3_PIN);
	case bit(CY3_PIN):
		return bit(CY5_PIN);
	case bit(CY5_PIN):
		return bit(CY7_PIN);
	case bit(CY7_PIN):
		return bit(CY2_PIN);
	}
	return 0;
}