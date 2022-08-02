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

Prescaler prescaler = Prescaler(1024);

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
        if (sys.time + prescaler.cts2us(TCNT1) > event_timestamp_us)
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

void start_timer1()
{
    // WGM mode 14 (fast PWM with ICR1 max)
    TCCR1A = bit(WGM11);
    TCCR1B = bit(WGM12) | bit(WGM13) | prescaler.TCCR1B_bits;

    ICR1 = UINT16_MAX;

    // Enable interrupts for overflow, match A, and match B
    TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B);
}

void setup_timer1(uint32_t frame_length_us, uint32_t frame_matchA_us, uint32_t frame_matchB_us)
{
    // We have three interrupts. Each of them must trigger a corresponding function
    // at most ONCE. However, the interrupts might occur more than once if the
    // interframe period is longer than 65k timer counts.

    // First, convert all intervals from microseconds to timer counts
    uint32_t frame_length_cts = prescaler.us2cts(frame_length_us);
    uint32_t frame_matchA_cts = prescaler.us2cts(frame_matchA_us);
    uint32_t frame_matchB_cts = prescaler.us2cts(frame_matchB_us);

    t1ovflow.n_cycles = 1;
    // We divide required #ticks in half until if fits into 16bit
    while (frame_length_cts >= UINT16_MAX)
    {
        frame_length_cts >>= 1;  // divide in half
        t1ovflow.n_cycles <<= 1; // double number of cycles
    }

    // Now let's find how many cycles we need for events A and B
    t1matchA.n_cycles = frame_matchA_cts / frame_length_cts;
    t1matchB.n_cycles = frame_matchB_cts / frame_length_cts;

    // Initialize the cycle counters
    t1ovflow.cycle = 0;
    t1matchA.cycle = 0;
    t1matchB.cycle = 0;

    // Pause timer1 to avoid inconsistent results
    GTCCR = bit(TSM) | bit(PSRSYNC);

    // Set timer1 registers ICR1, OCR1A and OCR1B to get precise interrupt timings
    // Interframe length is multiple of ICR1-long cycles
    ICR1 = (frame_length_cts > 0) ? frame_length_cts - 1 : 0;

    // Division remainder after several timer cycles
    OCR1A = frame_matchA_cts % frame_length_cts;
    OCR1B = frame_matchB_cts % frame_length_cts;

    // Subtract 1, but ensure it does not go below zero
    OCR1A = (OCR1A > 0) ? OCR1A - 1 : 0;
    OCR1B = (OCR1B > 0) ? OCR1B - 1 : 0; // Subtract 1, but ensure it does not go below zero

    // Update system time, then start timer a moment before overflow occurs
    sys.time += TCNT1 + 2;
    TCNT1 = ICR1 - 2;

    // Release the timer and let it run
    GTCCR = 0;
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
    sys.time += prescaler.cts2us(ICR1);

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
