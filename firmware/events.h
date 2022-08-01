#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>
#include <Arduino.h>
#include "sys_globals.h"

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