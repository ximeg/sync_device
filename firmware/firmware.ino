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
void send_ok() { Serial.print("OK\n"); }
void send_err() { Serial.print("ERR\n"); }
void send_err(const char *msg)
{
  Serial.print(msg);
  Serial.print("\n");
}

// Bit manipulation
uint8_t count_bits(uint8_t v)
{
  unsigned int c; // c accumulates the total bits set in v

  for (c = 0; v; v >>= 1)
  {
    c += v & 1;
  }
  return c;
}

uint8_t decode_shutter_bits(uint8_t rx_bits)
{
  uint8_t cy2_bit = (rx_bits & 1) > 0;
  uint8_t cy3_bit = (rx_bits & 2) > 0;
  uint8_t cy5_bit = (rx_bits & 4) > 0;
  uint8_t cy7_bit = (rx_bits & 8) > 0;
  return (cy2_bit << CY2_PIN) | (cy3_bit << CY3_PIN) | (cy5_bit << CY5_PIN) | (cy7_bit << CY7_PIN);
}

// TTL functions
void camera_pin_up() { CAMERA_PORT |= bit(CAMERA_PIN); }
void camera_pin_down() { CAMERA_PORT &= ~bit(CAMERA_PIN); }

void fluidic_pin_up() { FLUIDIC_PORT |= bit(FLUIDIC_PIN); }
void fluidic_pin_down() { FLUIDIC_PORT &= ~bit(FLUIDIC_PIN); }

void write_shutters(uint8_t value)
{
  SHUTTERS_PORT = (SHUTTERS_PORT & ~SHUTTERS_MASK) | value;
}

// Time functions
long elapsed_us() // returns number of microseconds elapsed since last call
{
  static uint32_t prev_time;
  int32_t delta = micros() - prev_time;
  if (delta < 0) // Check whether a clock overflow happened
  {
    delta += UINT32_MAX - 1;
  }
  prev_time = micros();
  return delta;
}

// Acquisition logic

void start_continuous_acq(uint32_t n_frames)
{
  sys.n_frames = n_frames;
  sys.n_acquired_frames = 0;

  // calculate time points for TTL triggers
  next_event.camera_TTL.up = micros();
  next_event.camera_TTL.down = next_event.camera_TTL.up + max(10, (sys.interframe_time_us >> 4));

  // change system state
  sys.status = STATUS::CONTINUOUS_ACQ_START;
}

void check_camera_events()
{
  switch (sys.status)
  {
  case STATUS::IDLE:
    break;

  case STATUS::CONTINUOUS_ACQ_START:
    if (t0 > next_event.camera_TTL.up)
    {
      camera_pin_up();
      next_event.camera_TTL.up += sys.interframe_time_us;
    }
    if (t0 > next_event.camera_TTL.down)
    {
      camera_pin_down();
      next_event.camera_TTL.down += sys.interframe_time_us;
    }
    break;

  default:
    break;
  }
}

/*************
SYSTEM STARTUP
*************/
void setup_output_ports()
{
  FLUIDIC_DDR |= bit(FLUIDIC_PIN);
  FLUIDIC_PORT &= ~bit(FLUIDIC_PIN);

  CAMERA_DDR |= bit(CAMERA_PIN);
  CAMERA_PORT &= ~bit(CAMERA_PIN);

  SHUTTERS_DDR |= SHUTTERS_MASK;
  SHUTTERS_PORT &= ~SHUTTERS_MASK;

  EVENT_LOOP_DDR |= bit(EVENT_LOOP_PIN);
}

// Setup serial port
void setup_UART()
{
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms

  // Wait until the serial port is ready
  while (!Serial)
  {
  }
  if (!sys.up)
  {
    Serial.flush();
    // Notify the host that we are ready
    Serial.print("Arduino is ready. Firmware version: ");
    Serial.print(VERSION);
    sys.up = true;
  }
}

// ------------------------------------------
// PROGRAM STARTING POINT
void setup()
{
  setup_output_ports();
  setup_UART();

  t0 = micros();
  elapsed_us();
}

/************
EVENT HANDLING
************/
void loop()
{
  // Save current time
  t0 = micros();
  check_UART_events();

  check_camera_events();

  // flip event loop pin - allows to monitor how fast loop() runs
  EVENT_LOOP_PIN_FLIP = bit(EVENT_LOOP_PIN);
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

  /* Set fluidics delay */
  case 'F':
    sys.fluidics_delay_us = data.fluidics_delay_us;
    send_ok();
    break;

  /* Set laser shutter and ALEX states */
  case 'L':
    if (data.Shutter.ALEX)
    {
      if (count_bits(data.Shutter.active) < 2)
      {
        // Less than two spectral channels - can't do ALEX!
        send_err("ALEX error: not enough channels");
        break;
      }
    }
    sys.shutter_active = decode_shutter_bits(data.Shutter.active);
    sys.shutter_idle = decode_shutter_bits(data.Shutter.idle);
    sys.ALEX_enabled = data.Shutter.ALEX;

    if (sys.status == STATUS::IDLE)
      write_shutters(sys.shutter_idle);

    if (sys.status == STATUS::CONTINUOUS_ACQ)
      write_shutters(sys.shutter_active);

    send_ok();
    break;

  /* Set interframe time delay */
  case 'I':
    if (data.interframe_time_us < 50)
    {
      send_err("Interframe time is too short");
      break;
    }
    sys.interframe_time_us = data.interframe_time_us;
    send_ok();
    break;

  /* Set strobe flash duration */
  case 'E':
    sys.strobe_duration_us = data.strobe_duration_us;
    send_ok();
    break;

  /* Set ALEX cycle delay (for timelapse with ALEX) */
  case 'A':
    sys.ALEX_cycle_delay_us = data.ALEX_cycle_delay_us;
    send_ok();
    break;

  /* Start continuous image acquisition */
  case 'C':
    start_continuous_acq(data.n_frames);
    send_ok();
    break;

  /* Start stroboscopic image acquisition */
  case 'S':
    if (sys.strobe_duration_us + 12000 > sys.interframe_time_us)
    {
      send_err("Not enough time to readout the sensor. Check strobe and interframe timings");
      break;
    }
    /* code */
    break;

  /* Stop image acquisition */
  case 'Q':
    /* code */
    send_ok();
    break;

  default:
    sys.status = STATUS::IDLE;
    break;
  }
}