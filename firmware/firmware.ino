#include <stdint.h>
#include <Arduino.h>

#ifndef _AVR_IOXXX_H_
#include <avr/iom328.h>
#endif

// #include "global_vars.h"

/***************************************
DEFINITIONS OF DATA TYPES AND STRUCTURES
***************************************/
#define uchar unsigned char
#define VERSION "0.2.0"

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1)    /* set alignment to 1 byte boundary */

typedef struct
{
  uint8_t addr;
  uint8_t value;
} Register;

typedef struct
{
  uint8_t active;
  uint8_t idle;
} Shutter;
Shutter g_shutter{0xF, 0};

typedef struct
{
  uint16_t timer_period_cts;
  uint16_t n_frames;            // number of frames
  uint16_t shutter_delay_cts;   // camera delay, in timer counts
  uint16_t injection_delay_cts; // injection delay, in timer counts
} Timer1;
Timer1 g_timer1{600, 7, 16, 48};

typedef struct
{
  uint16_t skip;
} Timelapse;
Timelapse g_timelapse{0};

typedef struct
{
  uint8_t mask;
} ALEX;
ALEX g_ALEX{0};

union Data
{
  struct
  {
    uint8_t cmd;

    // All members below share the same 9 bytes of memory
    // Each represents arguments of the command after which it is named
    union
    {
      Register R;  // register access (R/W)
      Shutter S;   // shutter control
      Timer1 T;    // timer configuration and E - exposure time
      Timelapse L; // L - timelapse
      ALEX A;      // A - ALEX mask for shutters
    };
  };
  uchar bytes[9];

} data;

#pragma pack(pop) /* restore original alignment from stack */

int charsRead;
bool system_is_up = false;

/************
PROGRAM LOGIC
************/

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms

  g_shutter.idle = 0;
}

// the loop function runs over and over again forever
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
        *((uchar *)data.R.addr) = data.R.value;
        break;

      case 'R':
      case 'r':
        // Read the value from the given register
        Serial.write(*((uchar *)data.R.addr));
        break;
      }
    }
  }
}
