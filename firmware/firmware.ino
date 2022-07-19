#define VERSION "0.2.0"

#include <stdint.h>
#include <Arduino.h>

#ifndef _AVR_IOXXX_H_
#include <avr/iom328.h>
#endif

#include "global_vars.h"

/************
HELPER FUNCTIONS
************/

/************
INTERRUPTS
************/

ISR(TIMER1_OVF_vect)
{
  // TOGGLE PORTC - it works!
  PORTC = 0xFF;

  // TODO: See what's the current status of the timer and select the corresponding action
}

ISR(TIMER1_COMPA_vect)
{
  PORTC = 0;
  PINB = 0xFF;
}

/************
SYSTEM STARTUP
************/
void setup()
{
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms

  // Setup ports
  DDRC = 0xFF;
  DDRB = 0xFF;

  // Start timer 1 and setup interrupt
  ICR1 = 6000;
  OCR1A = 150;
  TCCR1A = bit(WGM11);
  TCCR1B = bit(WGM12) | bit(WGM13) | bit(CS10) | bit(CS12);
  TIMSK1 = bit(TOIE1) | bit(OCIE1A);
  interrupts();
}

/************
EVENT HANDLING
************/
void loop()
{
  while (!Serial)
  {
    delay(1); // Wait until the serial port is ready
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
