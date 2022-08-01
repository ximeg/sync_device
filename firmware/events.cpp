#include "events.h"

#ifndef _AVR_IOXXX_H_
#include <avr/iom328.h>
#endif

typedef struct
{
    uint8_t bits;
    uint8_t bitshift; // << #ticks to covert them to microseconds
} Prescaler;

const Prescaler prescaler64 = {bit(CS10) | bit(CS11), 2};
const Prescaler prescaler256 = {bit(CS12), 4};
const Prescaler prescaler1024 = {bit(CS10) | bit(CS12), 6};

#define prescaler prescaler64

Event::Event(uint64_t event_timestamp_us, void (*event_handler)(), uint64_t repeat_every_us, uint32_t N_times)
{
    this->event_handler = event_handler;
    schedule(event_timestamp_us, repeat_every_us, N_times);
}

void Event::schedule(uint64_t event_timestamp_us)
{
    schedule(event_timestamp_us, repeat_every_us, N_times);
}
void Event::schedule(uint64_t event_timestamp_us, uint64_t repeat_every_us)
{
    schedule(event_timestamp_us, repeat_every_us, N_times);
}
void Event::schedule(uint64_t event_timestamp_us, uint64_t repeat_every_us, uint32_t N_times)
{
    event_counter = 0;
    event_ended = (event_timestamp_us) ? false : true;
    this->event_timestamp_us = event_timestamp_us;
    this->N_times = (N_times) ? N_times : UINT32_MAX;
    this->repeat_every_us = repeat_every_us;
}

void Event::check_event()
{
    if (!event_ended)
    {
        // maybe cache sys.time + TCNT1 cts to speedup computing? Check it with osci
        if (sys.time + (TCNT1 << prescaler.bitshift) > event_timestamp_us)
        {
            event_handler();

            // Do we need to reschedule it?
            if (repeat_every_us && (++event_counter < N_times))
            {
                event_timestamp_us += repeat_every_us;
            }
            else
            {
                event_ended = true;
            }
        }
    }
}

static volatile uint32_t frame_duration_us = 2000; // in microseconds
static volatile uint32_t event_A_us = 1000;        // in microseconds
static volatile uint32_t event_B_us = 500;         // in microseconds

typedef struct
{
    uint16_t cycle;
    uint16_t n_cycles;
} ISRcounter;

ISRcounter t1over;
ISRcounter t1matchA;
ISRcounter t1matchB;

// TODO: rename this?? Timer1 should always be running for sys.time. However, we need to keep in mind
// that some interrupts will be disabled to behave in different ways depending on sys.status
void setup_timer1()
{
    // We have three interrupts. Each of them must trigger a corresponding function
    // at most ONCE. However, the interrupts might occur more than once.

    // First, convert all intervals from microseconds to timer counts
    uint32_t frame_duration_cts = frame_duration_us >> prescaler.bitshift;
    uint32_t event_A_cts = event_A_us >> prescaler.bitshift;
    uint32_t event_B_cts = event_B_us >> prescaler.bitshift;

    t1over.n_cycles = 1;
    // We divide required #ticks in half until if fits into 16bit
    while (frame_duration_cts >= UINT16_MAX)
    {
        frame_duration_cts >>= 1; // divide in half
        t1over.n_cycles <<= 1;    // double
    }

    // Now let's find how many cycles we need for events A and B
    t1matchA.n_cycles = event_A_cts / frame_duration_cts;
    t1matchB.n_cycles = event_B_cts / frame_duration_cts;

    // Initialize the cycle counters
    t1over.cycle = 0;
    t1matchA.cycle = 0;
    t1matchB.cycle = 0;

    // Set the timer registers
    ICR1 = (frame_duration_cts > 0) ? frame_duration_cts - 1 : 0;
    OCR1A = event_A_cts % frame_duration_cts;
    OCR1B = event_B_cts % frame_duration_cts;
    OCR1A = (OCR1A > 0) ? OCR1A - 1 : 0;
    OCR1B = (OCR1B > 0) ? OCR1B - 1 : 0;

    // Reset and configure the timer
    TCNT1 = ICR1 - 2;
    TCCR1A = bit(WGM11);
    TCCR1B = bit(WGM12) | bit(WGM13) | prescaler.bits;

    // Enable interrupts for overflow, match A, and match B
    TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B);
}

void OVF_interrupt_handler()
{
    PINC = bit(PINC4);
}
void MatchA_interrupt_handler()
{
    PINC = bit(PINC3);
}
void MatchB_interrupt_handler()
{
    PINC = bit(PINC2);
}

ISR(TIMER1_OVF_vect)
{
    sys.time += ICR1 << prescaler.bitshift;

    if (sys.status == STATUS::IDLE)
        return;

    // Timer to call interrupt handler
    if (++t1over.cycle == t1over.n_cycles)
    {
        OVF_interrupt_handler();
        t1over.cycle = 0;
        t1matchA.cycle = 0;
        t1matchB.cycle = 0;
    }
}

ISR(TIMER1_COMPA_vect)
{
    if (sys.status == STATUS::IDLE)
        return;

    // Timer to call interrupt handler
    if (t1matchA.cycle++ == t1matchA.n_cycles)
    {
        MatchA_interrupt_handler();
    }
}

ISR(TIMER1_COMPB_vect)
{
    if (sys.status == STATUS::IDLE)
        return;

    // Timer to call interrupt handler
    if (t1matchB.cycle++ == t1matchB.n_cycles)
    {
        MatchB_interrupt_handler();
    }
}
