#include "sys_globals.h" // system time, state, settings, (pin assignments?)
#include "utils.h"       // convert bits, count bits, etc open/close shutters, send triggers.
#include "uart.h"        // setup_uart, parse_command, data definition
#include "triggers.h"    // port config, open/close shutters, send triggers.
#include "events.h"      // event loop - handling of event processing

void setup()
{
  setup_IO_ports();
  setup_UART();
  setup_timer1();
  // TODO: disable timer0? It might affect UART
}

Event event_fluidics_TTL_up = Event(0, fluidic_pin_up);
Event event_fluidics_TTL_dn = Event(0, fluidic_pin_down);

void loop()
{
  poll_UART();

  // Check and execute all scheduled events
  if (sys.status != STATUS::IDLE)
  {
    // Fluidics trigger
    event_fluidics_TTL_up.poll();
    event_fluidics_TTL_dn.poll();

    // event_start_imaging.poll();
  }

  // flip event loop pin - allows to monitor how fast `loop()` runs
  EVENT_LOOP_PIN_FLIP = bit(EVENT_LOOP_PIN);
}

/*





























#define MEM_IO_8bit(mem_addr) (*(volatile uint8_t *)(uintptr_t)(mem_addr))

typedef struct
{
  uint8_t bits;
  uint8_t bitshift; // << #ticks to covert them to microseconds
} Prescaler;

const Prescaler prescaler64 = {bit(CS10) | bit(CS11), 2};
const Prescaler prescaler256 = {bit(CS12), 4};
const Prescaler prescaler1024 = {bit(CS10) | bit(CS12), 6};

#define prescaler prescaler64

static volatile uint32_t frame_duration_us = 2000; // in microseconds
static volatile uint32_t event_A_us = 1000;        // in microseconds
static volatile uint32_t event_B_us = 500;         // in microseconds

// Setup serial port
void setup_UART()
{
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms

  // Wait until the serial port is ready
  while (!Serial)
  {
  }
  Serial.flush();
  // Notify the host that we are ready
  Serial.print("Arduino is ready. Firmware version: ");
  Serial.print(VERSION);
}

void check_UART_events()
{
  if (Serial.available())
  {
    if (Serial.readBytes(data.bytes, 5) == 5)
    {
      parse_UART_command();
    }
  }
}

typedef struct
{
  uint16_t cycle;
  uint16_t n_cycles;
} ISRcounter;

ISRcounter t1over;
ISRcounter t1matchA;
ISRcounter t1matchB;

void start_timer1()
{
  // We have three interrupts. Each of them must trigger a corresponding function
  // at most ONCE. However, the interrupts might occur more than once.

  // First, convert all intervals from microseconds to timer counts
  uint32_t frame_duration_cts = frame_duration_us >> prescaler.bitshift;
  uint32_t event_A_cts = event_A_us >> prescaler.bitshift;
  uint32_t event_B_cts = event_B_us >> prescaler.bitshift;

  t1over.n_cycles = 1;
  // We divide required #ticks in half until if fits into 16bit
  while (frame_duration_cts >= UINT16_MAX)
  {
    frame_duration_cts >>= 1; // divide in half
    t1over.n_cycles <<= 1;    // double
  }

  // Now let's find how many cycles we need for events A and B
  t1matchA.n_cycles = event_A_cts / frame_duration_cts;
  t1matchB.n_cycles = event_B_cts / frame_duration_cts;

  // Initialize the cycle counters
  t1over.cycle = 0;
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
  TCCR1B = bit(WGM12) | bit(WGM13) | prescaler.bits;

  // Enable interrupts for overflow, match A, and match B
  TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B);
}

void parse_UART_command()
{
  switch (data.cmd)
  {
  // Read register
case 'R':
Serial.write(MEM_IO_8bit(data.R.addr));
break;

// Write register
case 'W':
MEM_IO_8bit(data.R.addr) = data.R.value;
break;

case 'T':
frame_duration_us = data.interframe_time_us;
start_timer1();
break;

case 'A':
event_A_us = data.interframe_time_us;
start_timer1();
break;

case 'B':
event_B_us = data.interframe_time_us;
start_timer1();
break;
}
}

// ----------------------------------------------------
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
// ----------------------------------------------------

ISR(TIMER1_OVF_vect)
{
  PINC = bit(PINC5);

  // Timer to call interrupt handler
  if (++t1over.cycle == t1over.n_cycles)
  {
    OVF_interrupt_handler();
    t1over.cycle = 0;
    t1matchA.cycle = 0;
    t1matchB.cycle = 0;
  }
}

ISR(TIMER1_COMPA_vect)
{
  // Timer to call interrupt handler
  if (t1matchA.cycle++ == t1matchA.n_cycles)
  {
    MatchA_interrupt_handler();
  }
}

ISR(TIMER1_COMPB_vect)
{
  // Timer to call interrupt handler
  if (t1matchB.cycle++ == t1matchB.n_cycles)
  {
    MatchB_interrupt_handler();
  }
}

void setup()
{
  DDRC = bit(DDC5) | bit(DDC4) | bit(DDC3) | bit(DDC2);

  start_timer1();

  setup_UART();
}

void loop()
{
  check_UART_events();
}
*/