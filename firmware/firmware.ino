#define VERSION "0.2.0"

#include <stdint.h>
#include <Arduino.h>

#ifndef _AVR_IOXXX_H_
#include <avr/iom328.h>
#endif

#include "global_vars.h"

/***************
HELPER FUNCTIONS
****************/

uint8_t count_bits(uint8_t v)
{
  unsigned int c; // c accumulates the total bits set in v

  for (c = 0; v; v >>= 1)
  {
    c += v & 1;
  }
  return c;
}

inline void camera_pin_up() { CAMERA_PORT |= bit(CAMERA_PIN); }
inline void camera_pin_down() { CAMERA_PORT &= ~bit(CAMERA_PIN); }

inline void fluidic_pin_up() { FLUIDIC_PORT |= bit(FLUIDIC_PIN); }
inline void fluidic_pin_down() { FLUIDIC_PORT &= ~bit(FLUIDIC_PIN); }

inline void reset_timer1()
{
  // WGM mode 14, prescaler clk/1024 (datasheet tables 15-5 & 15-6)
  TCCR1A = bit(WGM11);
  TCCR1B = bit(WGM12) | bit(WGM13) | bit(CS10) | bit(CS12);

  // Disable timer 1 interrupts
  TIMSK1 = 0;
}

inline void start_timer1()
{
  // Read and set values for ICR1, OCR1A, OCR1B from global vars...
  ICR1 = g_timer1.timer_period_cts;
  OCR1A = g_timer1.shutter_delay_cts;
  OCR1B = OCR1A + min(ICR1 >> 3, 156); // 12% duty cycle, at most 10ms

  // Enable interrupts for overflow, match A, and match B
  TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B);

  // The first frame can either be NORMAL or ALEX
  system_status = is_alex_active() ? STATUS::ALEX_FRAME : STATUS::NORMAL_FRAME;
  interrupts();

  // Reset the timer and make sure it is not paused
  GTCCR = 0;
  TCNT1 = 0;
}

inline void write_shutters(uint8_t value)
{
  SHUTTERS_PORT &= ~SHUTTERS_MASK;
  SHUTTERS_PORT |= value;
}

uint8_t decode_shutter_bits(uint8_t rx_bits)
{
  uint8_t cy2_bit = (rx_bits & 1) > 0;
  uint8_t cy3_bit = (rx_bits & 2) > 0;
  uint8_t cy5_bit = (rx_bits & 4) > 0;
  uint8_t cy7_bit = (rx_bits & 8) > 0;
  return (cy2_bit << CY2_PIN) | (cy3_bit << CY3_PIN) | (cy5_bit << CY5_PIN) | (cy7_bit << CY7_PIN);
}

// Check whether ALEX mask contains more than one bit
bool is_alex_active()
{
  return (g_ALEX.mask & (g_ALEX.mask - 1)) != 0;
}

// Condition: acquired enough frames
inline void normal2idle()
{
  if (system_status == STATUS::NORMAL_FRAME)
  {
    if (g_timer1.n_frames > 0) // There is a frame limit
    {
      if (n_acquired_frames >= g_timer1.n_frames - 1) // We acquired enough frames!
      {
        n_acquired_frames = 0;
        system_status = STATUS::IDLE; // shutdown everything after the next cycle
      }
    }
  }
}

// Condition: timelapse is active
inline void normal2skip()
{
  if (system_status == STATUS::NORMAL_FRAME)
  {
    if (g_timelapse.skip > 0)
    {
      skipped_count = 0;
      system_status = STATUS::SKIP_FRAME; // We should skip the next frame
    }
  }
}

// Condition: enough frames skipped
inline void skip2normal()
{
  if (system_status == STATUS::SKIP_FRAME)
  {
    if (skipped_count >= g_timelapse.skip)
    {
      system_status = STATUS::NORMAL_FRAME; // Next frame is normal (or ALEX!?)
    }
  }
}

// Condition: enough frames acquired
inline void alex2idle()
{
  if (system_status == STATUS::ALEX_FRAME)
  {
    if (g_timer1.n_frames > 0) // There is a frame limit
    {
      uint16_t N = g_timer1.n_frames * count_bits(g_ALEX.mask);

      if (n_acquired_frames >= N) // We acquired enough frames!
      {
        n_acquired_frames = 0;
        system_status = STATUS::IDLE; // shutdown everything after the next cycle
      }
    }
  }
}

/**
 * @brief Evaluate all global variables and prepare the system to process the next timer event
 *
 * What we need to take into account?
 *  - current system status
 *  - do we have a frame limit? How many frames we already acquired?
 *  - Timelapse. is it active? how many frames we skipped?
 *  - ALEX. Is it active? What laser combinations are needed? What is the current one?
 */
void prepare_next_frame()
{
  normal2idle();
  normal2skip();
  skip2normal();
  alex2idle();
}

/*********
INTERRUPTS
**********/

// Timer overflow. This interrupt is called at the beginning of a frame and
// is usually responsible for control of the shutters that need to be
// opened/closed just a moment before the camera is triggered
// Additionally, it can be used to setup other things related to this frame
ISR(TIMER1_OVF_vect)
{
  uint8_t laser = 0;

  switch (system_status)
  {
  case STATUS::NORMAL_FRAME:
    write_shutters(g_shutter.active);
    // Check if ALEX is active?
    break;

  case STATUS::SKIP_FRAME:
    skipped_count++;
    write_shutters(0);
    break;

  case STATUS::ALEX_FRAME:
    // Cycle through spectral channels and set laser shutters
    while (!laser)
    {
      laser = (g_ALEX.mask >> alex_laser_i) & 1;
      if (laser)
      {
        write_shutters(decode_shutter_bits(bit(alex_laser_i)));
      }
      if (++alex_laser_i >= 4)
      {
        alex_laser_i = 0; // start the cycle over
      }
    }
    break;

  case STATUS::IDLE:
    write_shutters(g_shutter.idle);
    reset_timer1();
    Serial.println("DONE");
    break;

  default: // do nothing;
    break;
  }
}

// This interrupt sends raising edge of the camera trigger to start the acquisition
ISR(TIMER1_COMPA_vect)
{
  if (system_status != STATUS::SKIP_FRAME)
  {
    camera_pin_up();
    n_acquired_frames++;
  }
}

ISR(TIMER1_COMPB_vect)
{
  // Generate falling edge of the camera trigger.
  camera_pin_down();

  prepare_next_frame();
}

/************
SYSTEM STARTUP
************/
void setup()
{

  // ======================
  // TODO: this is a crutch
  // ======================
  g_timer1.n_frames = 2;
  g_shutter.idle = 0; // bit(CY2_PIN) | bit(CY7_PIN);
  g_timelapse.skip = 0;
  g_ALEX.mask = 0b1111;
  // END of TODO

  { // Setup serial port
    Serial.begin(2000000);
    Serial.setTimeout(10); // ms
  }

  { // Setup output ports
    FLUIDIC_DDR |= bit(FLUIDIC_PIN);
    FLUIDIC_PORT &= ~bit(FLUIDIC_PIN);

    CAMERA_DDR |= bit(CAMERA_PIN);
    CAMERA_PORT &= ~bit(CAMERA_PIN);

    SHUTTERS_DDR |= SHUTTERS_MASK;
    SHUTTERS_PORT &= ~SHUTTERS_MASK;
  }

  { // Configure timer 1 and setup interrupt
    reset_timer1();

    g_timer1.timer_period_cts = 15000;
    g_timer1.shutter_delay_cts = 1500;

    start_timer1();
  }
}

/************
EVENT HANDLING
************/
void loop()
{
  // Wait until the serial port is ready
  while (!Serial)
  {
  }

  if (!system_is_up)
  {
    Serial.flush();
    // Notify the host that we are ready
    Serial.print("Arduino is ready. Firmware version: ");
    Serial.println(VERSION);
    system_is_up = true;
  }

  // Parse the command
  if (Serial.available() > 0)
  {
    charsRead = Serial.readBytes(data.bytes, 3);
    if (charsRead == 3)
    {
      switch (data.cmd)
      {
      case 'A':
      case 'a':
        g_ALEX.mask = data.A.mask;
        break;

      case 'T':
      case 't':
        start_timer1();
        break;

      case 'W':
      case 'w':
        // Write the value to the register with given address
        MEM_IO_8bit(data.R.addr) = data.R.value;
        break;

      case 'R':
      case 'r':
        // Read the value from the given register
        Serial.write(MEM_IO_8bit(data.R.addr));
        break;

      // Remember the shutter state (keeps only 4 bits)
      case 'S':
      case 's':
        g_shutter.active = decode_shutter_bits(data.S.active);
        g_shutter.idle = decode_shutter_bits(data.S.idle);

        if (system_status == STATUS::IDLE)
          write_shutters(g_shutter.idle);

        if (system_status == STATUS::NORMAL_FRAME)
          write_shutters(g_shutter.active);

        break;
      }
    }
  }
}
