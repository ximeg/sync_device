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


void set_lasers(uint8_t laser)
{
	SHUTTERS_PORT = laser & SHUTTERS_MASK;
}

void lasers_off()
{
	SHUTTERS_PORT &= ~SHUTTERS_MASK;
}


uint8_t get_lowest_bit(uint8_t v)
{
	return v & -v;
}

uint8_t lowest_bit_position(uint8_t v)
{
	uint8_t r = 0;
	uint8_t lowest_bit = get_lowest_bit(v);
	while (lowest_bit >>= 1)
	{
		r++;
	}
	return r;
}

uint8_t highest_bit_position(uint8_t v)
{
	uint8_t r = 0;
	while (v >>= 1)
	{
		r++;
	}
	return r;
}

uint8_t next_laser()
{
	uint8_t laser_id = lowest_bit_position(sys.current_laser);
	uint8_t next_active = 0;
	
	while(1)
	{
		laser_id++;
		next_active = (sys.lasers_in_use >> laser_id) & 1UL;
		if (next_active)
		{
			return 1UL << laser_id;
		}
		if (laser_id == 8)
		{
			return get_lowest_bit(sys.lasers_in_use);
		}
	}
}

void reset_lasers()
{
	if (sys.ALEX_enabled)
	{
		sys.current_laser = get_first_laser();
	}
	else
	{
		sys.current_laser = sys.lasers_in_use;
	}
}

uint8_t get_first_laser()
{
	return get_lowest_bit(sys.lasers_in_use);
}
