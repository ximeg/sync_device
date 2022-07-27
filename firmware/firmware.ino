#define VERSION "0.3.0\n"

#include <stdint.h>
#include <Arduino.h>

#ifndef _AVR_IOXXX_H_
#include <avr/iom328.h>
#endif

#include "global_vars.h"

/***************
HELPER FUNCTIONS
****************/

// Serial port shortcuts
#define helper static inline void
helper send_ok() { Serial.print("OK\n"); }
helper send_err() { Serial.print("ERR\n"); }
helper send_err(const char *msg)
{
  Serial.print(msg);
  Serial.print("\n");
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

helper camera_pin_up() { CAMERA_PORT |= bit(CAMERA_PIN); }
helper camera_pin_down() { CAMERA_PORT &= ~bit(CAMERA_PIN); }

helper fluidic_pin_up() { FLUIDIC_PORT |= bit(FLUIDIC_PIN); }
helper fluidic_pin_down() { FLUIDIC_PORT &= ~bit(FLUIDIC_PIN); }

helper write_shutters(uint8_t value)
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

helper trigger_fluidics()
{
  fluidic_pin_up();
  delay(g_fluidics.fluidics_delay_ms);
  fluidic_pin_down();
}

helper reset_timer1()
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

helper start_timer1()
{
  // Set timer period to requested value, and at least 3 counts
  ICR1 = max(g_timer1.exp_time_n64us, 3);

  // Set delay between shutter and camera
  OCR1A = 10;                   // TODO: UNDEFINED BEHAVIOR!!!! Was g_timer1.shutter_delay_cts;
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
helper normal2idle()
{
  if (system_status == STATUS::CONTINUOUS_FRAME)
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

// Condition: enough frames acquired
helper alex2idle()
{
  if (system_status == STATUS::ALEX_FRAME)
  {
    if (g_timer1.n_frames > 0) // There is a frame limit
    {
      uint16_t N = g_timer1.n_frames * count_bits(g_shutter.active);

      if (n_acquired_frames >= N) // We acquired enough frames!
      {
        n_acquired_frames = 0;
        system_status = STATUS::IDLE; // shutdown everything after the next cycle
      }
    }
  }
}

// Condition: last spectral channel acquired and timelapse is active
helper alex2skip() /// TODO: we are not counting skipped frames! This function is incorrect
{
  if (system_status == STATUS::ALEX_FRAME && g_timer1.timelapse_delay_s > 0)
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
helper skip2alex() /// TODO: we are not counting skipped frames! This function is incorrect
{
  if (system_status == STATUS::SKIP_FRAME && g_shutter.ALEX)
  {
    if (skipped_count >= g_timer1.timelapse_delay_s)
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
  case STATUS::CONTINUOUS_FRAME:
    write_shutters(g_shutter.active);
    break;

  case STATUS::SKIP_FRAME:
    skipped_count++;
    write_shutters(0);
    // Since we delayed the camera, we should wait more here ???????
    delay(g_timer1.timelapse_delay_s);
    break;

  case STATUS::ALEX_FRAME:
    // Cycle through spectral channels and set laser shutters
    if (g_shutter.ALEX)
    {

      while (!laser)
      {
        laser = (g_shutter.active >> alex_laser_i) & 1;
        if (laser)
        {
          write_shutters(decode_shutter_bits(bit(alex_laser_i)));
        }
        if (++alex_laser_i > alex_last_laser)
        {
          alex_laser_i = 0; // start the cycle over
        }
      }
    }
    else
    {
      write_shutters(g_shutter.active);
      alex_laser_i = 0;
    }
    break;

  case STATUS::IDLE:
    reset_timer1();

    // Since we delayed the camera, we should wait more here ???????
    delay(10); // enough to read out a frame
    write_shutters(g_shutter.idle);
    Serial.print("DONE\n");
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
  // Setup output ports
  FLUIDIC_DDR |= bit(FLUIDIC_PIN);
  FLUIDIC_PORT &= ~bit(FLUIDIC_PIN);

  CAMERA_DDR |= bit(CAMERA_PIN);
  CAMERA_PORT &= ~bit(CAMERA_PIN);

  SHUTTERS_DDR |= SHUTTERS_MASK;
  SHUTTERS_PORT &= ~SHUTTERS_MASK;

  // Configure timer 1 and setup interrupt
  reset_timer1();

  setup_UART();
}

// Setup serial port
helper setup_UART()
{
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms

  // Wait until the serial port is ready
  while (!Serial)
  {
  }
  if (!system_is_up)
  {
    Serial.flush();
    // Notify the host that we are ready
    Serial.print("Arduino is ready. Firmware version: ");
    Serial.print(VERSION);
    system_is_up = true;
  }
}

/************
EVENT HANDLING
************/
void loop()
{
  parse_command();

  // check_timer_states_transitions_etc();
}

helper parse_command()
{
  if (Serial.available() > 0)
  {
    charsRead = Serial.readBytes(data.bytes, 9);
    if (charsRead == 9)
    {
      switch (data.cmd)
      {
        /*case 'A':
        case 'a':
          g_ALEX.mask = data.A.mask;
          g_ALEX.alternate = data.A.alternate;
          alex_last_laser = 0;
          while (data.A.mask >>= 1)
          {
            alex_last_laser++;
          }
          break;*/

      /* Read register */
      case 'R':
      case 'r':
        // Read the value from the given register
        Serial.write(MEM_IO_8bit(data.R.addr));
        break;

      /* Write register */
      case 'W':
      case 'w':
        // Write the value to the register with given address
        MEM_IO_8bit(data.R.addr) = data.R.value;
        break;

      /* Set fluidics delay */
      case 'F':
      case 'f':
        // Save the fluidics time delay
        g_fluidics.fluidics_delay_ms = data.F.fluidics_delay_ms;
        send_ok();
        break;

      /* Set laser shutter states */
      case 'L':
      case 'l':
        // Ensure that more than one spectral channel is selected if ALEX is on
        if (data.L.ALEX)
        {
          if (data.L.active & (data.L.active - 1) == 0)
          {
            // Less than two spectral channels - can't do ALEX!
            send_err("ALEX error: not enough channels");
            break;
          }
        }

        g_shutter.active = decode_shutter_bits(data.L.active);
        g_shutter.idle = decode_shutter_bits(data.L.idle);
        g_shutter.ALEX = data.L.ALEX;

        if (system_status == STATUS::IDLE)
          write_shutters(g_shutter.idle);

        if (system_status == STATUS::CONTINUOUS_FRAME)
          write_shutters(g_shutter.active);

        send_ok();
        break;

      /* Start acquisition */
      case 'C':
      case 'c':
      case 'S':
      case 's':
        if (data.T.exp_time_n64us < 5)
        {
          // don't bother, the period is too short
          send_err("ACQ error: exposure time is too short");
          break;
        }

        reset_timer1();
        write_shutters(g_shutter.idle);

        memcpy(&g_timer1, &data.T, sizeof(g_timer1));

        trigger_fluidics();

        n_acquired_frames = 0;

        if (data.cmd == 'c' || data.cmd == 'C')
        {
          system_status = STATUS::CONTINUOUS_FRAME;
        }
        else
        {
          system_status = g_shutter.ALEX ? STATUS::ALEX_FRAME : STATUS::STROBO_FRAME;
        }

        start_timer1();
        fluidic_pin_down();
        send_ok();
        break;

      /* Change the exposure time (on the fly) */
      case 'E':
      case 'e':
        g_timer1.exp_time_n64us = data.T.exp_time_n64us;
        start_timer1();
        send_ok();
        break;

      /* Stop timer, if running */
      case 'Q':
      case 'q':
        reset_timer1();
        write_shutters(g_shutter.idle);
        send_ok();
        break;
      }
    }
  }
}