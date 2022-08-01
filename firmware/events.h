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

class Event
{
private:
    void (*event_handler)();
    uint64_t event_timestamp_us;
    uint64_t repeat_every_us;
    uint32_t N_times;
    uint32_t event_counter;

public:
    Event(uint64_t event_timestamp_us, void (*event_handler)(), uint64_t repeat_every_us = 0, uint32_t N_times = 0);
    void schedule(uint64_t event_timestamp_us);
    void schedule(uint64_t event_timestamp_us, uint64_t repeat_every_us);
    void schedule(uint64_t event_timestamp_us, uint64_t repeat_every_us, uint32_t N_times);
    bool event_ended;
    void check_event();
};

#endif // EVENTS_H