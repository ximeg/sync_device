#include <iostream>
#include <chrono>
using namespace std::chrono;

#include <stdint.h>

static uint64_t t0 = duration_cast<milliseconds>(
                         system_clock::now().time_since_epoch())
                         .count();

uint64_t sys_time()
{
    return duration_cast<milliseconds>(
               system_clock::now().time_since_epoch())
               .count() -
           t0;
}

class Event
{
private:
    void (*event_handler)();
    uint32_t event_timestamp_us;
    uint32_t repeat_every_us;
    uint32_t number_events;
    uint32_t times_fired;

public:
    Event(uint32_t event_timestamp_us, void (*event_handler)(), uint32_t repeat_every_us = 0, uint32_t number_events = 0);
    void schedule(uint32_t event_timestamp_us, uint32_t repeat_every_us = 0, uint32_t number_events = 0);
    bool event_ended;
    void check_event();
};

Event::Event(uint32_t event_timestamp_us, void (*event_handler)(), uint32_t repeat_every_us, uint32_t number_events)
{
    this->event_handler = event_handler;
    schedule(event_timestamp_us, repeat_every_us, number_events);
}

void Event::schedule(uint32_t event_timestamp_us, uint32_t repeat_every_us, uint32_t number_events)
{
    times_fired = 0;
    event_ended = false;
    this->event_timestamp_us = event_timestamp_us;
    this->number_events = (number_events) ? number_events : UINT32_MAX;
    this->repeat_every_us = repeat_every_us;
}

void Event::check_event()
{
    if (!event_ended)
    {
        if (sys_time() > event_timestamp_us)
        {
            event_handler();

            // Do we need to reschedule it?
            if (repeat_every_us && (++times_fired < number_events))
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

void up()
{
    std::cout << "trigger up at " << sys_time() << std::endl;
}

void down()
{
    std::cout << "trigger down at " << sys_time() << std::endl;
}

int main()
{
    Event trigger_up = Event(50, up, 200, 4);
    Event trigger_down = Event(70, down, 200, 4);

    do
    {
        trigger_up.check_event();
        trigger_down.check_event();
    } while (!trigger_down.event_ended);
    std::cout << sys_time() << std::endl;
}