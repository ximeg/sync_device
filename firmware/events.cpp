#include "events.h"

// Timer 1 prescaler configuration. See AtMega328 datasheet table 15-6
class Prescaler
{
private:
    int8_t leftshift_cts2us;
    uint16_t factor;

public:
    Prescaler(uint16_t factor = 64);
    uint8_t TCCR1B_bits;
    uint32_t (*cts2us)(uint32_t cts); // counts to microseconds
    uint32_t (*us2cts)(uint32_t us);  // microseconds to counts
};

Prescaler::Prescaler(uint16_t factor)
{
    this->factor = factor;
    switch (factor)
    {
    // 62.5ns cycle (16x per microsecond)
    case 1:
        TCCR1B_bits = bit(CS10);
        cts2us = [](uint32_t cts)
        { return cts >> 4; };

        us2cts = [](uint32_t us)
        { return us << 4; };
        break;

    // 500ns cycle (2x per microsecond)
    case 8:
        TCCR1B_bits = bit(CS11);
        cts2us = [](uint32_t cts)
        { return cts >> 1; };

        us2cts = [](uint32_t us)
        { return us << 1; };
        break;

    // 4us cycle
    case 64:
        TCCR1B_bits = bit(CS10) | bit(CS11);
        cts2us = [](uint32_t cts)
        { return cts << 2; };

        us2cts = [](uint32_t us)
        { return us >> 2; };
        break;

    // 16us cycle
    case 256:
        TCCR1B_bits = bit(CS12);
        cts2us = [](uint32_t cts)
        { return cts << 4; };

        us2cts = [](uint32_t us)
        { return us >> 4; };
        break;

    // 64us cycle
    case 1024:
        TCCR1B_bits = bit(CS10) | bit(CS12);
        cts2us = [](uint32_t cts)
        { return cts << 6; };

        us2cts = [](uint32_t us)
        { return us >> 6; };
        break;
    }
};

Prescaler presc = Prescaler(1024);

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

void Event::poll()
{
    if (!event_ended)
    {
        // maybe cache sys.time + TCNT1 cts to speedup computing? Check it with osci
        if (sys.time + presc.cts2us(TCNT1) > event_timestamp_us)
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

static volatile uint32_t event_A_us = 1000; // in microseconds
static volatile uint32_t event_B_us = 500;  // in microseconds

typedef struct
{
    uint16_t cycle;
    uint16_t n_cycles;
} ISRcounter;

ISRcounter t1ovflow;
ISRcounter t1matchA;
ISRcounter t1matchB;

// TODO: rename this?? Timer1 should always be running for sys.time. However, we need to keep in mind
// that some interrupts will be disabled to behave in different ways depending on sys.status

void setup_timer1()
{
    // We have three interrupts. Each of them must trigger a corresponding function
    // at most ONCE. However, the interrupts might occur more than once if the
    // interframe period is longer than 65k timer counts.

    // First, convert all intervals from microseconds to timer counts
    uint32_t frame_duration_cts = presc.us2cts(sys.interframe_time_us);
    uint32_t event_A_cts = presc.us2cts(event_A_us);
    uint32_t event_B_cts = presc.us2cts(event_B_us);

    t1ovflow.n_cycles = 1;
    // We divide required #ticks in half until if fits into 16bit
    while (frame_duration_cts >= UINT16_MAX)
    {
        frame_duration_cts >>= 1; // divide in half
        t1ovflow.n_cycles <<= 1;  // double
    }

    // Now let's find how many cycles we need for events A and B
    t1matchA.n_cycles = event_A_cts / frame_duration_cts;
    t1matchB.n_cycles = event_B_cts / frame_duration_cts;

    // Initialize the cycle counters
    t1ovflow.cycle = 0;
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
    TCCR1B = bit(WGM12) | bit(WGM13) | presc.TCCR1B_bits;

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
    sys.time += presc.cts2us(ICR1);

    if (sys.status == STATUS::IDLE)
        return;

    // Timer to call interrupt handler
    if (++t1ovflow.cycle == t1ovflow.n_cycles)
    {
        OVF_interrupt_handler();
        t1ovflow.cycle = 0;
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
