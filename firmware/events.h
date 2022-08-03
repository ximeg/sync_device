/**
 * @file events.h
 * @author Roman Kiselev (roman.kiselev@stjude.org)
 * @brief Event handling routines and timer1 interrupts
 * @version 0.3
 * @date 2022-08-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef EVENTS_H
#define EVENTS_H

#include "sys_globals.h"
#include "triggers.h"

/**
 * @brief Configure and start timer 1 in the default mode. If system is IDLE, it is responsible only for the system time.
 */
void start_timer1();

/**
 * @brief Configure timer1 to control camera and laser shutter triggers with custom timings.
 *
 * @param frame_length_us  Frame duration in microseconds. `frame_start_event()` is called once at the start of each frame
 * @param frame_matchA_us  Time point to call `frame_matchA_event()` once relative to the frame start
 * @param frame_matchB_us  Time point to call `frame_matchB_event()` once relative to the frame start
 * @param reset            Whether the timer should be reset (TCNT1 modified)
 */
void setup_timer1(uint32_t frame_length_us, uint32_t frame_matchA_us, uint32_t frame_matchB_us, bool reset = true);

/**
 * @brief A scheduled, potentially repetitive, event. Calls event handler when it's time to do so.
 *
 * The event class represents a scheduled call to the `event_handler` function. If timestamp is zero,
 * the event is disabled. The `event_handler` will be called once if the current system time passes the event
 * timestamp. After that either `event_ended` will become *true*, or the event will get automatically
 * rescheduled (if `repeat_every_us` is set). If also `N_times` is provided, the event will be repeated
 * given number of times before it ends.
 *
 * Use this class in combination with IO port settings to create waveforms.
 *
 * You have to call `poll()` from the main system `loop()` to ensure prompt event execution.
 */
class Event
{
private:
    void (*event_handler)();
    uint64_t event_timestamp_us;
    uint64_t repeat_every_us;
    uint32_t N_times;
    uint32_t event_counter;

public:
    Event(
        uint64_t event_timestamp_us,  // Timestamp when the event is supposed to happen. If 0, event is disabled
        void (*event_handler)(),      // Function to call when the event happens
        uint64_t repeat_every_us = 0, // How often to repeat this event? Zero means no repeats
        uint32_t N_times = 1          // How many times to call the event
    );

    // Reschedule an existing event
    void schedule(uint64_t event_timestamp_us);
    void schedule(uint64_t event_timestamp_us, uint64_t repeat_every_us);
    void schedule(uint64_t event_timestamp_us, uint64_t repeat_every_us, uint32_t N_times);

    // Indicates whether the event ended (i.e. was triggered given number of times)
    bool event_ended;

    // Poll this event - call often in the main event processing loop. Calls handler if the event is due.
    void poll();
};

void start_continuous_imaging();

inline Event event_fluidics_TTL_up = Event(0, fluidic_pin_up);
inline Event event_fluidics_TTL_dn = Event(0, fluidic_pin_down);

inline Event event_start_continuous_acq = Event(0, nullptr);

#endif // EVENTS_H