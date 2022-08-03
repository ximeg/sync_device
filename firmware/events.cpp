#include "events.h"
#include "timer1.h"

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
        if (sys.time + cts2us(TCNT1) > event_timestamp_us)
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

void start_continuous_imaging()
{
    sys.n_acquired_frames = 0;

    timer1.pause();
    // timings
    timer1.set_ICR1(2500);  //(sys.interframe_time_us);
    timer1.set_OCR1A(500);  //(min(sys.interframe_time_us >> 1, LASER_SHUTTER_DELAY));
    timer1.set_OCR1B(1000); //(min((sys.interframe_time_us >> 1) + 100, LASER_SHUTTER_DELAY + 100));
    // events
    timer1.overfl_handler = []()
    {
        write_shutters(sys.shutter_active);
    };
    timer1.matchA_handler = camera_pin_up;
    timer1.matchB_handler = []()
    {
        camera_pin_down();
        if (++sys.n_acquired_frames > sys.n_frames && sys.n_frames > 0)
        {
            stop_acquisition();
        }
    };
    timer1.run();
}

void stop_acquisition()
{
    sys.status = STATUS::IDLE;

    timer1.overfl_handler = noop;
    timer1.matchA_handler = noop;
    timer1.matchB_handler = noop;

    write_shutters(sys.shutter_idle);
    camera_pin_down();
    fluidic_pin_down();
}

/*

void setup_timer1(uint32_t frame_length_us, uint32_t frame_matchA_us, uint32_t frame_matchB_us, bool reset)
{
    // We have three interrupts. Each of them must trigger a corresponding function
    // at most ONCE. However, the interrupts might occur more than once if the
    // interframe period is longer than 65k timer counts.

    // First, convert all intervals from microseconds to timer counts
    uint32_t frame_length_cts = us2cts(frame_length_us);
    uint32_t frame_matchA_cts = us2cts(frame_matchA_us);
    uint32_t frame_matchB_cts = us2cts(frame_matchB_us);

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
    // Interframe length is multiple of ICR1-long cycles. ICR1 has to be greater than 0
    ICR1 = (frame_length_cts > 1) ? frame_length_cts - 1 : 1;

    // Division remainder after several timer cycles
    OCR1A = frame_matchA_cts % frame_length_cts;
    OCR1B = frame_matchB_cts % frame_length_cts;

    // Subtract 1, but ensure it is at least 1
    OCR1A = (OCR1A > 1) ? OCR1A - 1 : 1;
    OCR1B = (OCR1B > 1) ? OCR1B - 1 : 1;

    // Ensure OCR1A < OCR1B < ICR1
    OCR1B = (OCR1B >= ICR1) ? ICR1 - 1 : OCR1B;
    OCR1A = (OCR1A >= OCR1B) ? OCR1B - 1 : OCR1A;

    if (reset)
    {
        // Update system time, then start timer a moment before overflow occurs
        sys.time += TCNT1 + 2;
        TCNT1 = ICR1 - 2;
    }

    // Release the timer and let it run
    GTCCR = 0;
}

// --------------------------------------------------------------


// --------------------------------------------------------------

void frame_start_event()
{
    switch (sys.status)
    {
    case STATUS::IDLE:
        write_shutters(sys.shutter_idle);
        camera_pin_down();
        fluidic_pin_down();
        break;

    case STATUS::CONTINUOUS_ACQ_PREP:
        break;

    case STATUS::CONTINUOUS_ACQ:
        write_shutters(sys.shutter_active);
        break;

    case STATUS::CONTINUOUS_ACQ_POST:
        break;

    default:
        break;
    }
}

void frame_matchA_event()
{
    if (sys.status == STATUS::IDLE)
        return;

    camera_pin_up();
}

void frame_MatchB_event()
{
    if (sys.status == STATUS::IDLE)
        return;

    camera_pin_down();

    // evaluate situation and decide where to move next
    switch (sys.status)
    {
    case STATUS::CONTINUOUS_ACQ_PREP: // this happens only once
        cli();
        setup_timer1(sys.interframe_time_us,
                     LASER_SHUTTER_DELAY,
                     LASER_SHUTTER_DELAY + (sys.interframe_time_us >> 2),
                     false); // longer timer period without resetting
        sys.status = STATUS::CONTINUOUS_ACQ;
        sei();
        break;

    case STATUS::CONTINUOUS_ACQ:
        if (++sys.n_acquired_frames > sys.n_frames && sys.n_frames > 0)
        {
            cli();
            // one more short frame (discarded later)
            uint32_t discard_frame_length = min(sys.interframe_time_us, CAMERA_READOUT) + LASER_SHUTTER_DELAY;
            setup_timer1(discard_frame_length,
                         LASER_SHUTTER_DELAY,
                         LASER_SHUTTER_DELAY + (sys.interframe_time_us >> 2),
                         false);                      // longer timer period without resetting
            sys.status = STATUS::CONTINUOUS_ACQ_POST; // one more short frame
            sei();
        }
        break;

    case STATUS::CONTINUOUS_ACQ_POST:
        sys.status = STATUS::IDLE; // IDLE forces shutters to close
        break;

    default:
        break;
    }
}
*/