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

void poll_events()
{
    event_camera_TTL_up.poll();
    event_camera_TTL_dn.poll();
    event_fluidics_TTL_dn.poll();
    event_fluidics_TTL_dn.poll();
}

/*
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

*/