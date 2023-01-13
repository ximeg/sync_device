#include "timers.h"
#include "triggers.h"


static uint16_t t3_cycle = 0;
static uint16_t t3_N_OVF_cycles = 1;
static bool last_burst_frame = false;

void set_timer3_period(uint32_t us)
{
	t3_cycle = 0;
	t3_N_OVF_cycles = 1;
	// We divide required #counts in half until if fits into 16bit
	uint32_t cts = us2cts(us);
	while (cts >= 65535)
	{
		cts >>= 1;             // divide in half
		t3_N_OVF_cycles <<= 1; // double number of cycles
	}
	ICR3 = cts - 1;
}

void start_acq()
{
	// Set timing intervals for TC1
	OCR1A = us2cts((sys.shutter_delay_us + 1));
	OCR1B = us2cts(sys.exp_time_us);
	OCR1C = us2cts((sys.exp_time_us - 2)) + OCR1A;
	ICR1 = OCR1C + us2cts(sys.cam_readout_us) - 1;
	
	// Set timing interval for TC3 (it may need multiple cycles)
	set_timer3_period(sys.acq_period_us);
	
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
	lasers_on();
}


// ---------------------------
// INTERRUPTS
// ---------------------------

ISR(TIMER1_COMPA_vect)
{
	bitSet(CAMERA_PORT, CAMERA_PIN);
}

ISR(TIMER1_COMPB_vect)
{
	lasers_off();
}

ISR(TIMER1_COMPC_vect)
{
	bitClear(CAMERA_PORT, CAMERA_PIN);
	
	// Additional logic - preparation for the next frame

	// This code is relevant only for ALEX
	sys.current_laser = next_laser();
	// Did we reach the last frame in the burst?
	uint8_t first_laser = sys.lasers_in_use & -sys.lasers_in_use;
	// Cy2 because we already moved on to the next laser
	last_burst_frame = (sys.current_laser == first_laser);
}

ISR(TIMER1_OVF_vect)
{
	if(last_burst_frame)
	{
		TCCR1B = bit(WGM12) | bit(WGM13);
	}
	else
	{
		lasers_on();
	}
}


ISR(TIMER3_OVF_vect)
{
	t3_cycle++;
	if (t3_cycle >= t3_N_OVF_cycles)
	{
		if ((++sys.n_acquired_frames < sys.n_frames) || (sys.n_frames == 0))
		{
			// Reset the prescaler to sync timers
			GTCCR |= PSRSYNC;
	
			t3_cycle = 0;

			// Open laser shutters
			lasers_on();

			// Start timer 1
			TCCR1B |= TCCR1B_prescaler_bits;
		}
		else
		{
			// Acquisition is finished. Stop timers and reset frame counters
			TCCR1B &= ~TCCR1B_prescaler_bits;
			TCCR3B &= ~TCCR1B_prescaler_bits;
			
			sys.n_acquired_frames = 0;
			sys.n_frames = 0;
		}
	}
}
