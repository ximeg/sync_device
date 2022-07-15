/*
TODO:
 - troubleshoot individual functions in a MWE
 - then put them together
 - hardcode timer values to something that makes sense, e.g. 15ms injection,
   then 8 ms delay, and 10 ms pulses - easy to see on osci.
*/

#include <stdint.h>
#include <Arduino.h>
#include <avr/iom328.h> // Comment this line out to compile and upload!

#define VERSION "0.2.0"

/***************************************
DEFINITIONS OF DATA TYPES AND STRUCTURES
***************************************/
#pragma pack(push) /* push current alignment to stack */
#pragma pack(1)    /* set alignment to 1 byte boundary */

union Data
{
  // 3-byte data packet used for normal communication
  struct
  {
    uint8_t cmd;       // equals to 'r', 'R', 'w' or 'W'
    uint8_t reg_addr;  // address of register to read/write
    uint8_t reg_value; // value of the register to write
  };
  // extended data packet used to setup timer 1
  struct
  {
    uint8_t t_cmd;             // equals to 't' or 'T'
    uint16_t timer_period_cts; // timer period, in timer counts
    union
    {
      struct
      {
        uint16_t n_frames;      // number of frames
        uint16_t cam_delay_cts; // camera delay, in timer counts
        uint16_t inj_delay_cts; // injection delay, in timer counts
      };
      uint8_t bytes_extra[6]; // 6-byte extension
    };
  };

  uint8_t bytes[9];
} data;

#pragma pack(pop) /* restore original alignment from stack */

volatile uint8_t charsRead;
volatile bool up = false;

volatile uint8_t timer1_status = 0;
volatile uint16_t timer_period_cts = 0;

/************
  INTERRUPTS
************/

enum T1STATUS
{
  STOPPED = 0,
  SHUTTER_OPENING = 1,
  ACQUISITION = 2
};

// ISR
ISR(TIMER1_OVF_vect)
{
  switch (timer1_status)
  {
  case T1STATUS::SHUTTER_OPENING:
    ++timer1_status;

    // Fluidic signal goes down
    PORTC &= ~bit(PORTC4);

    // Emit signal to shutters
    DDRC |= bit(DDC3) | bit(DDC2) | bit(DDC1) | bit(DDC0);

    // Timer 1 runs in fast PWM mode, we need to set the proper timings
    ICR1 = timer_period_cts;

    // Set the timer to account for the camera/shutter delay
    TCNT1 = timer_period_cts - data.cam_delay_cts;

    // Enable OC1A output
    DDRB |= bit(DDB1);
    break;

  case T1STATUS::ACQUISITION:; // count pulses, stop the timer when reach n_frames
    break;
  }
}

/************
PROGRAM LOGIC
************/

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms

  // Enable shutter output on PORTC, pins 0-3
  PORTC = 0;
  DDRC = 0;

  // Enable fluidics trigger output on PORTC, pin 4
  DDRC |= bit(DDC4);
  PORTC &= ~bit(PORTC4);

  // Disable camera trigger output on PB1 and set it to zero (timer 1)
  PORTB &= ~bit(PORTB1);
  DDRB &= ~bit(DDB1);
}

void loop()
{
  while (!Serial)
  {
    delay(1); // Wait until the serial port is ready
  }

  if (!up)
  {
    Serial.flush();
    // Notify the host that we are ready
    Serial.print("Arduino is ready. Firmware version: ");
    Serial.println(VERSION);
    up = true;
  }

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
        *((uint8_t *)data.reg_addr) = data.reg_value;
        break;

      case 'R':
      case 'r':
        // Read the value from the given register
        Serial.write(*((uint8_t *)data.reg_addr));
        break;

      case 'T': // start timer 1

        // OC1A goes down
        // _reset_OC1A();

        // _start_timer1()
        // Send trigger to fluidic system
        PORTC |= bit(PORTC4);

        // Start timer 1 - waiting for fludic mixing
        timer1_status = T1STATUS::SHUTTER_OPENING;

        TCCR1A = bit(WGM11) | bit(COM1A1);
        TCCR1B = bit(WGM13) | bit(WGM12) | bit(CS12) | bit(CS10);

        ICR1 = data.inj_delay_cts;
        timer_period_cts = data.timer_period_cts;
        OCR1A = max(timer_period_cts >> 4, 1);

        // Enable the overflow interrupt (TIMER1_OVF_vect)
        TIMSK1 |= bit(TOIE1);
        interrupts();

        break;
      case 't': // stop timer 1
        Serial.println("Stopping timer 1");
        // _stop_timer1()
        break;
      }
    }
  }
}
