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
        if (sys_time() > event_timestamp_us)
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
    Event trigger_up = Event(0, up, 200, 4); // 0 means deactivated, but configured
    Event trigger_down = Event(270, down, 200, 4);

    trigger_up.schedule(250);
    do
    {
        trigger_up.check_event();
        trigger_down.check_event();
    } while (!trigger_down.event_ended);
    std::cout << sys_time() << std::endl;
}