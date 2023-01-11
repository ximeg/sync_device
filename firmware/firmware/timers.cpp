#include "timers.h"
#include "triggers.h"

#define SET_FRAME_DURATION(us) (ICR1 = us2cts(us) - 1)
#define SET_CAM_RAISING_EDGE(us) (OCR1A = us2cts(us))
#define SET_CAM_FALLING_EDGE(us) (OCR1C = us2cts(us))
#define SET_LASER_FALLING_EDGE(us) (OCR1B = us2cts(us))


void start_acq()
{
	// Set timing intervals for TC1
	OCR1A = us2cts(sys.shutter_delay_us);
	OCR1B = us2cts(sys.exp_time_us);
	OCR1C = us2cts(sys.exp_time_us) + OCR1A;
	ICR1 = OCR1C + 1;
	
	// Set timing interval for TC3
	ICR3 = us2cts(sys.acq_period_us) - 1;
	
	// Pause and sync timers
	GTCCR |= bit(TSM) | bit(PSRSYNC);
	
	// Configure timers 1 and 3
	TCCR1A = bit(WGM11); // WGM mode 14 (fast PWM with ICR1 max)
	TCCR1B = bit(WGM12) | bit(WGM13) | TCCR1B_prescaler_bits;

	TCCR3A = bit(WGM31);
	TCCR3B = bit(WGM32) | bit(WGM33) | TCCR1B_prescaler_bits;

	// Enable interrupts for overflow, match A, and match B
	TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B) | bit(OCIE1C);
	TIMSK3 = bit(TOIE3);

	// Let the timers go!
	bitClear(GTCCR, TSM);
	
	// open laser shutters
	write_shutters(0x0f);

}


// ---------------------------
// INTERRUPTS
// ---------------------------

ISR(TIMER1_OVF_vect)
{
	// Stop the timer (TC3 will reactivate it later)
	TCCR1B = bit(WGM12) | bit(WGM13);
}

ISR(TIMER1_COMPA_vect)
{
	bitSet(CAMERA_PORT, CAMERA_PIN);
}

ISR(TIMER1_COMPB_vect)
{
	write_shutters(0x00);
}

ISR(TIMER1_COMPC_vect)
{
	bitClear(CAMERA_PORT, CAMERA_PIN);
}

ISR(TIMER3_OVF_vect)
{
	// Reset the prescaler to sync timers
	GTCCR |= PSRSYNC;
	
	// Start timer 1
	TCCR1B |= TCCR1B_prescaler_bits;
	
	// Open laser shutters
	write_shutters(0x0f);
}