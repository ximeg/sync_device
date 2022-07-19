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
    // Th
    union
    {
      // R/W - register access
      Register R;
      Register W;

      // S - shutter control
      Shutter S;

      // T - timer configuration and E - exposure time
      Timer1 T;

      // L - timelapse
      Timelapse L;

      // A - ALEX mask
      ALEX A;
    };
  };
  uchar bytes[9];

} data;

#pragma pack(pop) /* restore original alignment from stack */

// Bytes in the data frame
struct Bytes
{
  uchar cmd;
  uchar addr;
  uchar value;
};

// Data frame consists of bytes but can be represented as text
union DataFrame
{
  struct Bytes bytes;
  uchar text[4];
};

union DataFrame df;
int charsRead;
bool up = false;

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
    charsRead = Serial.readBytes(df.text, 3);
    if (charsRead == 3)
    {
      switch (df.bytes.cmd)
      {
      case 'W':
      case 'w':
        // Write the value to the register with given address
        *((uchar *)df.bytes.addr) = df.bytes.value;
        break;

      case 'R':
      case 'r':
        // Read the value from the given register
        Serial.write(*((uchar *)df.bytes.addr));
        break;
      }
    }
  }
}
