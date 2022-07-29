#define VERSION "0.3.0\n"
#include <stdint.h>
#include <Arduino.h>

#ifndef _AVR_IOXXX_H_
#include <avr/iom328.h>
#endif

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1)    /* set alignment to 1 byte boundary */

// Address and value of register for read/write operations
typedef struct
{
  uint8_t addr;
  uint8_t value;
} Register;

// Laser shutter states - in active and idle mode
typedef struct
{
  uint8_t active;
  uint8_t idle;
  bool ALEX;
} LaserShutter;

// Data packet for serial communication
union Data
{
  struct
  {
    uint8_t cmd;

    // All members below share the same chunk of memory
    union
    {
      Register R;                   // register access (R/W)
      int32_t fluidics_delay_us;    // fluidics injection delay. If negative, happens before imaging
      LaserShutter Shutter;         // laser shutter control and ALEX on/off
      uint32_t interframe_time_us;  // time between frames in any imaging mode
      uint32_t strobe_duration_us;  // duration of laser flash in stroboscopic mode
      uint32_t ALEX_cycle_delay_us; // duration of delay between ALEX cycles
      uint32_t n_frames;            // number of frames to acquire
    };
  };

  uint8_t bytes[9];
} data;

#pragma pack(pop) /* restore original alignment from stack */

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
  /* Read register */
  case 'R':
    Serial.write(MEM_IO_8bit(data.R.addr));
    break;

  /* Write register */
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
