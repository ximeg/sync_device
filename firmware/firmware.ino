#include <stdint.h>

// Uncomment for IntelliSense to work
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

/************
  INTERRUPTS
************/

// ISR
ISR(TIMER1_OVF_vect)
{
  PORTC = 0XFF - PORTC;
}

/************
PROGRAM LOGIC
************/

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms

  // Timer 1 output
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);

  DDRC = 0xFF;
  PORTC = 0;
}

// the loop function runs over and over again forever
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
        TCCR1A = bit(WGM11) | bit(COM1A1);
        TCCR1B = bit(WGM13) | bit(WGM12) | bit(CS12) | bit(CS10);

        ICR1 = data.timer_period_cts;
        OCR1A = max(data.timer_period_cts >> 4, 1);

        PORTC = 0xFF;

        // Enable the overflow interrupt (TIMER1_OVF_vect)
        TIMSK1 |= bit(TOIE1);
        interrupts();

        break;
      case 't': // stop timer 1
        Serial.println("Stopping timer 1");
        break;
      }
    }
  }
}
