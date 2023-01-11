#include "timers.h"
#include "triggers.h"

#define SET_FRAME_DURATION(us) (ICR1 = us2cts(us) - 1)
#define SET_CAM_RAISING_EDGE(us) (OCR1A = us2cts(us))
#define SET_LASER_FALLING_EDGE(us) (OCR1B = us2cts(us))
#define SET_CAM_FALLING_EDGE(us) (OCR1C = us2cts(us))

void init_timer1()
{
	// Default settings
	SET_FRAME_DURATION(4000);
	SET_CAM_RAISING_EDGE(300);
	SET_LASER_FALLING_EDGE(450);
	SET_CAM_FALLING_EDGE(3950);
	
	
	TCNT1 = ICR1 - 1;
	t0 = micros();

	// WGM mode 14 (fast PWM with ICR1 max)
	TCCR1A = bit(WGM11);
	TCCR1B = bit(WGM12) | bit(WGM13) | TCCR1B_prescaler_bits;

	// Enable interrupts for overflow, match A, and match B
	TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B) | bit(OCIE1C);
}



// ---------------------------
// INTERRUPTS
// ---------------------------

// Start new frame by opening laser shutters
ISR(TIMER1_OVF_vect)
{
	write_shutters(0x0f);  // should be enable_shutters(); or something similar instead
	t0 = micros();
}

// Trigger camera (raising edge)
ISR(TIMER1_COMPA_vect)
{
	bitSet(CAMERA_PORT, CAMERA_PIN);
}

// Close laser shutters
ISR(TIMER1_COMPB_vect)
{
	write_shutters(0x00);
}

// Generate camera trigger falling edge
// Evaluate system status 
ISR(TIMER1_COMPC_vect)
{
	bitClear(CAMERA_PORT, CAMERA_PIN);
	n_acquired_frames++;
	
	// pause timer1 until we reach the end of the frame (reactivated in the main event loop)
	// This should happen only if we are in the timelapse mode, i.e. interframe time > exposure time
	if (interframe_time_us > cts2us(ICR1))
	{
		if (n_acquired_frames >= n_frames)
		{
			n_acquired_frames = 0;
			TCCR1B = bit(WGM12) | bit(WGM13);	
		}
	}
}
