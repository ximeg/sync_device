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

inline void write_camera_pin(int value)
{
  if (value)
    CAMERA_PORT |= bit(CAMERA_PIN);
  else
    CAMERA_PORT &= ~bit(CAMERA_PIN);
}

inline void write_fluidic_pin(int value)
{
  if (value)
    FLUIDIC_PORT |= bit(FLUIDIC_PIN);
  else
    FLUIDIC_PORT &= ~bit(FLUIDIC_PIN);
}

inline void reset_timer1(){
  // Pause timer 1
  GTCCR = PSRSYNC | TSM;

  // WGM mode 14, prescaler clk/1024 (datasheet tables 15-5 & 15-6)
  TCCR1A = bit(WGM11);
  TCCR1B = bit(WGM12) | bit(WGM13) | bit(CS10) | bit(CS12);

  // Enable interrupts for overflow, match A, and match B
  TIMSK1 = bit(TOIE1) | bit(OCIE1A) | bit(OCIE1B);

  // Reset the timer
  TCNT1 = 0;
}

inline void start_timer1(){
  // Read and set values for ICR1, OCR1A, OCR1B from global vars...

  // Let it run
  GTCCR = 0;
  system_status = STATUS::NORMAL_FRAME;
}

inline void write_shutters(uint8_t value)
{
  SHUTTERS_PORT &= ~SHUTTERS_MASK;
  SHUTTERS_PORT |= value;
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
    write_shutters(0);
    break;
  
  case STATUS::ALEX_FRAME:
    // fancy BGR switching here... Later...
    break;

  case STATUS::LAST_FRAME:
    write_shutters(g_shutter.idle);
    break;

  default: // do nothing;
    break;
  }

  // Check number of acquired frames
  if (n_acquired_frames >= g_timer1.n_frames)
  {
    system_status = STATUS::LAST_FRAME;
    write_fluidic_pin(1);
  }

}

// This interrupt sends raising edge of the camera trigger to start the acquisition
ISR(TIMER1_COMPA_vect)
{
  switch (system_status)
  {
  case STATUS::NORMAL_FRAME:
  case STATUS::ALEX_FRAME:
    write_camera_pin(1);
    n_acquired_frames++;
    break;
  
  default: // do nothing;
    break;
  }
}

// Generate falling edge of the camera trigger. Nothing else, really
ISR(TIMER1_COMPB_vect)
{
  write_camera_pin(0);
}


/************
SYSTEM STARTUP
************/
void setup()
{
  {// Setup serial port
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms
  }

  {// Setup output ports
  FLUIDIC_DDR |= bit(FLUIDIC_PIN);
  FLUIDIC_PORT &= ~bit(FLUIDIC_PIN);

  CAMERA_DDR |= bit(CAMERA_PIN);
  CAMERA_PORT &= ~bit(CAMERA_PIN);

  SHUTTERS_DDR |= SHUTTERS_MASK;
  SHUTTERS_PORT &= ~SHUTTERS_MASK;
  }
  
  {// Configure timer 1 and setup interrupt
  reset_timer1();
  ICR1 = 10000; // timer period (overflow interrupt)
  OCR1A = 1000; // first match interrupt
  OCR1B = 3000; // second match interrupt
 
  interrupts();
  start_timer1();
  }


  g_timer1.n_frames = 4;  // TODO: FIXME
  g_shutter.idle = bit(CY2_PIN) | bit(CY7_PIN);
}

/************
EVENT HANDLING
************/
void loop()
{
  // Wait until the serial port is ready
  while (!Serial){} 

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
      }
    }
  }
}
