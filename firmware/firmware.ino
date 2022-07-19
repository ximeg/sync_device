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

// Check whether ALEX mask contains more than one bit
bool is_alex_active()
{
  return (g_ALEX.mask & (g_ALEX.mask - 1)) != 0;
}

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

inline void write_shutters(uint8_t value)
{
  SHUTTERS_PORT = (SHUTTERS_PORT & ~SHUTTERS_MASK) | value;
}

uint8_t decode_shutter_bits(uint8_t rx_bits)
{
  uint8_t cy2_bit = (rx_bits & 1) > 0;
  uint8_t cy3_bit = (rx_bits & 2) > 0;
  uint8_t cy5_bit = (rx_bits & 4) > 0;
  uint8_t cy7_bit = (rx_bits & 8) > 0;
  return (cy2_bit << CY2_PIN) | (cy3_bit << CY3_PIN) | (cy5_bit << CY5_PIN) | (cy7_bit << CY7_PIN);
}

inline void reset_timer1()
{
  // WGM mode 14, prescaler clk/1024 (datasheet tables 15-5 & 15-6)
  TCCR1A = bit(WGM11);
  TCCR1B = bit(WGM12) | bit(WGM13) | bit(CS10) | bit(CS12);

  // Disable timer 1 interrupts
  TIMSK1 = 0;
  TIFR1 = 0xFF;

  // Reset the system to the standard state
  system_status = STATUS::IDLE;
  n_acquired_frames = 0;
  skipped_count = 0;
  alex_laser_i = 0;
}

inline void start_timer1()
{
  // Set timer period to requested value, and at least 3 counts
  ICR1 = max(g_timer1.timer_period_cts, 3);

  // Set delay between shutter and camera
  OCR1A = g_timer1.shutter_delay_cts;
  OCR1A = min(OCR1A, ICR1 - 2); // Sanity check: make sure OCR1A is lower than ICR1

  // 12.5% duty cycle, at least 1 count, at most 10ms
  OCR1B = OCR1A + max(min(ICR1 >> 3, 156), 1);
  OCR1B = min(OCR1B, ICR1 - 1); // Sanity check: make sure OCR1B is lower than ICR1

  // Clear interrupt flags
  TIFR1 = 0xFF;

  // Enable interrupts for overflow, match A, and match B
  TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B);

  // Reset the timer and make sure it is not paused
  TCNT1 = ICR1 - 2;
  GTCCR = 0;
  interrupts();
}

/**********
TRANSITIONS
***********/

// Condition: acquired enough frames
inline void normal2idle()
{
  if (system_status == STATUS::NORMAL_FRAME)
  {
    if (g_timer1.n_frames > 0) // There is a frame limit
    {
      if (n_acquired_frames >= g_timer1.n_frames) // We acquired enough frames!
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
  if (system_status == STATUS::SKIP_FRAME && !is_alex_active())
  {
    if (skipped_count >= g_timelapse.skip)
    {
      system_status = STATUS::NORMAL_FRAME; // Next frame is normal
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

// Condition: last spectral channel acquired and timelapse is active
inline void alex2skip()
{
  if (system_status == STATUS::ALEX_FRAME && g_timelapse.skip > 0)
  {

    if (n_acquired_frames > 0) // Make sure we don't skip from the start
    {
      if (alex_laser_i == 0) // it resets at the end of the ALEX cycle
      {
        skipped_count = 0;
        system_status = STATUS::SKIP_FRAME; // We should skip the next frame
      }
    }
  }
}

// Condition: enough frames skipped and ALEX is active
inline void skip2alex()
{
  if (system_status == STATUS::SKIP_FRAME && is_alex_active())
  {
    if (skipped_count >= g_timelapse.skip)
    {
      system_status = STATUS::ALEX_FRAME; // Next frame is ALEX
    }
  }
}

/**
 * Evaluate all global variables and prepare the system to process the next timer event
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
  alex2skip();
  skip2alex();
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
    reset_timer1();

    // Since we delayed the camera, we should wait more here
    delayMicroseconds(g_timer1.shutter_delay_cts << 6);
    write_shutters(g_shutter.idle);
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

  // Decide what to do during the next timer cycle.
  prepare_next_frame();
}

/************
SYSTEM STARTUP
************/
void setup()
{
  // Setup serial port
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms

  // Setup output ports
  FLUIDIC_DDR |= bit(FLUIDIC_PIN);
  FLUIDIC_PORT &= ~bit(FLUIDIC_PIN);

  CAMERA_DDR |= bit(CAMERA_PIN);
  CAMERA_PORT &= ~bit(CAMERA_PIN);

  SHUTTERS_DDR |= SHUTTERS_MASK;
  SHUTTERS_PORT &= ~SHUTTERS_MASK;

  // Configure timer 1 and setup interrupt
  reset_timer1();
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
        if (g_timer1.timer_period_cts == 0)
          break; // don't bother, period is zero

        // Read the remaining data from the packet
        charsRead = Serial.readBytes(data.bytes + 3, 6);
        if (charsRead == 6)
        {
          reset_timer1();
          write_shutters(g_shutter.idle);

          Serial.println("START");
          memcpy(&g_timer1, &data.T, sizeof(g_timer1));

          // Send trigger to fluidic system
          fluidic_pin_up();

          // Wait for given number of 64us intervals (up to 1023!)
          uint16_t d = g_timer1.injection_delay_cts;
          if (d > 2)
            delayMicroseconds((d - 2) << 6);

          n_acquired_frames = 0;

          // The first frame can either be NORMAL or ALEX
          system_status = is_alex_active() ? STATUS::ALEX_FRAME : STATUS::NORMAL_FRAME;

          start_timer1();
          fluidic_pin_down();
          break;
        }

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

      case 'L':
      case 'l':
        g_timelapse.skip = data.L.skip;
        break;

      case 'E':
      case 'e':
        g_timer1.timer_period_cts = data.T.timer_period_cts;
        start_timer1();
        break;

      case 'Q':
      case 'q':
        reset_timer1();
        write_shutters(g_shutter.idle);
        break;
      }
    }
  }
}
