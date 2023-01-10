#include "timers.h"
#include "triggers.h"

#define SET_FRAME_DURATION(us) (ICR1 = us2cts(us) - 1)
#define SET_CAM_RAISING_EDGE(us) (OCR1A = us2cts(us))
#define SET_CAM_FALLING_EDGE(us) (OCR1C = us2cts(us))
#define SET_LASER_FALLING_EDGE(us) (OCR1B = us2cts(us))

void setup_timer1()
{
	// Default settings
	SET_FRAME_DURATION(9000);
	SET_CAM_RAISING_EDGE(1000);
	SET_LASER_FALLING_EDGE(2000);
	SET_CAM_FALLING_EDGE(4000);
	TCNT1 = ICR1 - 1;
		
	// WGM mode 14 (fast PWM with ICR1 max)
	TCCR1A = bit(WGM11);
	TCCR1B = bit(WGM12) | bit(WGM13) | TCCR1B_prescaler_bits;

	// Enable interrupts for overflow, match A, and match B
	TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B) | bit(OCIE1C);
}



// ---------------------------
// INTERRUPTS
// ---------------------------

ISR(TIMER1_OVF_vect)
{
	write_shutters(0x0f);  // should be enable_shutters(); or something similar instead
	// reset system timer - NOT IMPLEMENTED
}

ISR(TIMER1_COMPA_vect)
{
	camera_pin_up();
}

ISR(TIMER1_COMPB_vect)
{
	write_shutters(0x00);
}

ISR(TIMER1_COMPC_vect)
{
	camera_pin_down();
}
