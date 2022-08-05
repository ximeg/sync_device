#include "timer1.h"

void setup_timer1()
{
    // WGM mode 14 (fast PWM with ICR1 max)
    TCCR1A = bit(WGM11);
    TCCR1B = bit(WGM12) | bit(WGM13) | TCCR1B_bits;

    // Use the max period by default
    ICR1 = UINT16_MAX;

    // Enable interrupts for overflow, match A, and match B
    TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B);
}

void reset_timer1()
{
    uint8_t old_SREG = SREG;
    cli();
    sys.time += TCNT1;
    TCNT1 = 0;
    SREG = old_SREG;
}

// --------------------------------------------------------------

// interrupt 13, first event during the frame
ISR(TIMER1_OVF_vect)
{
    // Update system time
    sys.time += ICR1;

    // DEBUG: confirm the timer is running
    PINC = bit(PINC0);

    // poll events
}

// interrupt 11, second event during the frame
ISR(TIMER1_COMPA_vect)
{
    // poll events
}

// interrupt 12, third event during the frame
ISR(TIMER1_COMPB_vect)
{
    // poll events
}
