#include "timer1.h"

Timer1::Timer1(uint32_t ICR1_us, uint32_t OCR1A_us, uint32_t OCR1B_us)
{
    set_ICR1(ICR1_us);
    set_OCR1A(OCR1A_us);
    set_OCR1B(OCR1B_us);

    // WGM mode 14 (fast PWM with ICR1 max)
    TCCR1A = bit(WGM11);
    TCCR1B = bit(WGM12) | bit(WGM13) | TCCR1B_bits;

    // Enable interrupts for overflow, match A, and match B
    TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B);
}

void Timer1::reset()
{
    cli();
    t1ovflow.cycle = 0;
    t1matchA.cycle = 0;
    t1matchB.cycle = 0;
    TCNT0 = ICR1 - 2;
    sei();
}

void Timer1::set_ICR1(uint32_t us)
{
    cli();
    ICR1_32 = us;
    uint32_t cts = us2cts(us);

    t1ovflow.n_cycles = 1;
    // We divide required #counts in half until if fits into 16bit
    while (cts >= UINT16_MAX)
    {
        cts >>= 1;               // divide in half
        t1ovflow.n_cycles <<= 1; // double number of cycles
    }

    ICR1 = cts - 1;
    reset();
    sei();
}

void Timer1::set_OCR1A(uint32_t us)
{
    cli();
    OCR1A_32 = us;
    uint32_t cts = us2cts(us);

    t1matchA.n_cycles = cts / (ICR1 + 1);

    OCR1A = cts % ICR1;
    reset();
    sei();
}

void Timer1::set_OCR1B(uint32_t us)
{
    cli();
    OCR1B_32 = us;
    uint32_t cts = us2cts(us);

    t1matchB.n_cycles = cts / (ICR1 + 1);

    OCR1B = cts % ICR1;
    reset();
    sei();
}

void Timer1::rewind_time(int32_t us)
{
    cli();
    int32_t cts;
    if (us >= 0)
    {
        cts = us2cts(us);
    }
    else
    {
        cts = -us2cts(-us);
    }

    uint16_t N = ICR1 + 1;
    t1ovflow.cycle += cts / N;

    uint16_t adj = abs(cts) % N;

    TCNT1 = (cts >= 0) ? TCNT1 + adj : TCNT1 - adj;
    sei();
}

uint32_t Timer1::get_time()
{
    return ICR1 * t1ovflow.cycle + TCNT1;
}

void Timer1::pause()
{
    GTCCR = bit(TSM) | bit(PSRSYNC);
}

void Timer1::run()
{
    GTCCR = 0;
}

// --------------------------------------------------------------

ISR(TIMER1_COMPA_vect) // interrupt 11
{
    // It's time to call interrupt handler
    if (t1matchA.cycle++ == t1matchA.n_cycles)
    {
        timer1.matchA_handler();
    }
}

ISR(TIMER1_COMPB_vect) // interrupt 12
{
    // It's time to call interrupt handler
    if (t1matchB.cycle++ == t1matchB.n_cycles)
    {
        timer1.matchB_handler();
    }
}

ISR(TIMER1_OVF_vect) // interrupt 13
{
    // It's time to call interrupt handler
    if (++t1ovflow.cycle >= t1ovflow.n_cycles)
    {
        timer1.overfl_handler();
        t1ovflow.cycle = 0;
        t1matchA.cycle = 0;
        t1matchB.cycle = 0;
    }
}