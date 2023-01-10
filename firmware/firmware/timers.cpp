#include "timers.h"
#include "triggers.h"

void setup_timer1()
{
	// Default settings
	ICR1 = us2cts(1000) - 1;
	OCR1A = us2cts(40);
	OCR1B = us2cts(100);
	OCR1C = us2cts(200);
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
