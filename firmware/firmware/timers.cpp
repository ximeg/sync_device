#include "timers.h"
#include "triggers.h"
#include "uart.h"

static uint16_t t3_cycle = 0;
static uint16_t t3_N_OVF_cycles = 1;
static bool last_burst_frame = false;

void setup_timer3(uint32_t us)
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

void start_continuous_acq(uint32_t n_frames /*= 0*/)
{
	stop_acq();
	sys.status = CONT_ACQ;
	sys.n_frames = n_frames;

	// Set timing interval for TC1 (controls lasers and camera)
	OCR1A = 0;		// Camera raising edge
	OCR1C = us2cts(sys.exp_time_us) >> 2;	// Camera falling edge
	OCR1C = (OCR1C > 0) ? OCR1C : 1;
	ICR1 = us2cts(sys.exp_time_us) - 1;	// Overflow - next frame
	
	// Pause and sync timers
	GTCCR |= bit(TSM) | bit(PSRSYNC);

	// Configure timers 1 and 3
	TCCR1A = bit(WGM11); // WGM mode 14 (fast PWM with ICR1 max)
	TCCR1B = bit(WGM12) | bit(WGM13) | TCCR1B_prescaler_bits;

	// Enable interrupts for overflow, match A, and match B
	TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1C);

	// Let the timers go!
	bitClear(GTCCR, TSM);

	// open laser shutters
	lasers_on();
	
	// Use TC3 for the first frame???
}

void start_stroboscopic_acq(uint32_t n_frames /*= 0*/)
{
	stop_acq();
	sys.status = STRB_ACQ;
	sys.n_frames = n_frames;

	// Set timing interval for TC1 (controls lasers and camera)
	OCR1A = us2cts((sys.shutter_delay_us + 1));		// Camera raising edge
	OCR1B = us2cts(sys.exp_time_us);				// Laser off
	OCR1C = us2cts((sys.exp_time_us - 2)) + OCR1A;	// Camera falling edge
	ICR1 = OCR1C + us2cts(sys.cam_readout_us) - 1;	// Overflow - next frame

	// Set timing interval for TC3 (it may need multiple cycles)
	setup_timer3(sys.acq_period_us);

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
	if (sys.status == STRB_ACQ){
		lasers_off();
	}
}

ISR(TIMER1_COMPC_vect)
{
	bitClear(CAMERA_PORT, CAMERA_PIN);

	// Additional logic - preparation for the next frame
	if (sys.status == STRB_ACQ)
	{
		if (sys.ALEX_enabled)
		{
			// Activate the next laser
			sys.current_laser = next_laser();
			// Cy2 because we already moved on to the next laser
			last_burst_frame = (sys.current_laser == get_first_laser());
		}
		else
		{
			last_burst_frame = true;
		}
	}
}

ISR(TIMER1_OVF_vect)
{
	if (sys.status == STRB_ACQ)
	{
		if(last_burst_frame)
		{
			// Pause timer 1
			TCCR1B = bit(WGM12) | bit(WGM13);
		}
		else
		{
			// Open shutter for the next laser within the burst (ALEX only)
			lasers_on();
		}
	}
}


ISR(TIMER3_OVF_vect)
{
	t3_cycle++;
	if (t3_cycle >= t3_N_OVF_cycles) // count # of overflow events for long intervals
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
			stop_acq();
			UART_tx("DONE\n");
		}
	}
}

void stop_acq()
{
	// Turn off lasers and camera
	lasers_off();
	bitClear(CAMERA_PORT, CAMERA_PIN);

	// Reset everything
	sys.status = IDLE;
	reset_lasers();
	sys.n_acquired_frames = 0;
	t3_cycle = 0;
	t3_N_OVF_cycles = 1;
	last_burst_frame = false;

	// Stop timers and reset frame counters
	TCCR1B &= ~TCCR1B_prescaler_bits;
	TCCR3B &= ~TCCR1B_prescaler_bits;
	
	// Reset timers
	TCNT1 = 0;
	TCNT3 = 0;
}