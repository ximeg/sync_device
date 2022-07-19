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

inline void set_fluidic_pin(int value)
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
}


/************
INTERRUPTS
************/

ISR(TIMER1_OVF_vect)
{
  // toggle shutter
  PINC |= 0xF;
}


ISR(TIMER1_COMPA_vect)
{
write_camera_pin(1);
}

ISR(TIMER1_COMPB_vect)
{
  // Generate falling edge of the camera trigger
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

  SHUTTERS_DDR |= bit(CY2_PIN) | bit(CY3_PIN) | bit(CY5_PIN) | bit(CY7_PIN);
  SHUTTERS_PORT &= ~(bit(CY2_PIN) | bit(CY3_PIN) | bit(CY5_PIN) | bit(CY7_PIN));
  }
  
  {// Configure timer 1 and setup interrupt
  reset_timer1();
  ICR1 = 20000; // timer period (overflow interrupt)
  OCR1A = 3000; // first match interrupt
  OCR1B = 4000; // second match interrupt
 
  interrupts();
  start_timer1();
  }
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
        *((uint8_t *)data.R.addr) = data.R.value;
        break;

      case 'R':
      case 'r':
        // Read the value from the given register
        Serial.write(*((uint8_t *)data.R.addr));
        break;
      }
    }
  }
}
