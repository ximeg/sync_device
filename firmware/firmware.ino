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

  // Enable interrupts for overflow, match A, and match B
  TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B);

  // Reset the timer and make sure it is not paused
  GTCCR = 0;
  TCNT1 = 0;

  system_status = STATUS::NORMAL_FRAME;
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

/************
INTERRUPTS
************/

// Timer overflow. This interrupt is called at the beginning of a frame and
// is usually responsible for control of the shutters that need to be
// opened/closed just a moment before the camera is triggered
// Additionally, it can be used to setup other things related to this frame
ISR(TIMER1_OVF_vect)
{
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
    // fancy BGR switching here... Later...
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

  // Evaluate the current situation and prepare for the next frame
  if (n_acquired_frames >= g_timer1.n_frames - 1) // We acquired enough frames
  {
    n_acquired_frames = 0;
    system_status = STATUS::IDLE; // shutdown everything after in the next cycle
    fluidic_pin_up();
    return;
  }

  if (skipped_count == 0 && g_timelapse.skip > 0) // && ALEX sequence has finished
  {
    system_status = STATUS::SKIP_FRAME; // We should skip the next frame
    return;
  }

  switch (system_status)
  {
  case STATUS::SKIP_FRAME:
    // count skipped frames
    if (skipped_count >= g_timelapse.skip)
    {
      skipped_count = 0;
      system_status = STATUS::NORMAL_FRAME; // Next frame is normal (or ALEX!?)
    };
    break;
  }
}

/************
SYSTEM STARTUP
************/
void setup()
{
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
    ICR1 = 10000; // timer period (overflow interrupt)
    OCR1A = 1000; // first match interrupt
    OCR1B = 3000; // second match interrupt

    interrupts();
    start_timer1();
  }

  g_timer1.n_frames = 10; // TODO: FIXME
  g_shutter.idle = bit(CY2_PIN) | bit(CY7_PIN);
  g_timelapse.skip = 3;
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
