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

void setup_timer1();

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

#endif // EVENTS_H