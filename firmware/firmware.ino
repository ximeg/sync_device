#include <stdint.h>
#include <Arduino.h>
#include <avr/iom328.h> // Comment this line out to compile and upload!

#define VERSION "0.2.0"

/***************************************
DEFINITIONS OF DATA TYPES AND STRUCTURES
***************************************/

// Bitmask for shutter outputs in PORTC
#define SHUTTER_MASK 0b00001111

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1)    /* set alignment to 1 byte boundary */

// TODO: this structure repeats itself - boilerplate
struct T1
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
} timer1_cfg;

union Data
{
  // 3-byte data packet used for normal communication
  struct
  {
    uint8_t cmd; // TODO: it's redundant
    union
    {
      struct
      {
        uint8_t reg_addr;  // address of register to read/write
        uint8_t reg_value; // value of the register to write
      };
      struct
      {
        uint8_t shutter_running; // state of shutters when running
        uint8_t shutter_idle;    // state of shutters when idle
      };
    };
  };
  // extended data packet used to setup timer 1
  struct
  {
    uint8_t t_cmd;             // equals to 't' or 'T'. TODO: redundant
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

enum T1STATUS
{
  STOPPED = 0,
  SHUTTER_OPENING = 1,
  ACQUISITION = 2,
  LAST_FRAME = 3,
};

volatile uint8_t charsRead;
volatile bool up = false;

volatile uint8_t shutter_running = 0xFF & SHUTTER_MASK;
volatile uint8_t shutter_idle = 0;

volatile uint8_t timer1_status = T1STATUS::STOPPED;
volatile uint16_t timer_period_cts = 0;

volatile uint16_t n_acquired_frames = 0;

/************
  INTERRUPTS
************/

// ISR
ISR(TIMER1_OVF_vect)
{
  switch (timer1_status)
  {
  case T1STATUS::SHUTTER_OPENING:
    ++timer1_status;

    // Fluidic signal goes down
    PORTC &= ~bit(PORTC4);

    write_shutter();

    // Make sure that the timer output is set to zero
    reset_OC1A();

    start_timer1();
    break;

  case T1STATUS::ACQUISITION:; // count pulses, stop the timer when reach n_frames
    if (++n_acquired_frames >= timer1_cfg.n_frames && timer1_cfg.n_frames > 0)
    {
      ++timer1_status;

      while (TCNT1 < OCR1A)
      {
        ; // Wait until timer reaches output compare match to have a clean signal
      }

      // Disable OC1A timer output to avoid extra trigger at the end of the last frame
      DDRB &= ~bit(DDB1);
    }
    break;

  case T1STATUS::LAST_FRAME:
    // Disable timer 1 interrupts and stop timer
    TIMSK1 &= ~bit(TOIE1);
    stop_timer1();
    break;
  }

  PINB = bit(PINB5);
}

/************
PROGRAM LOGIC
************/

const unsigned char TCCR1A_default = bit(COM1A1) | bit(WGM11);
const unsigned char TCCR1B_default = bit(WGM13) | bit(WGM12) | bit(CS12) | bit(CS10);

void write_shutter()
{
  PORTC &= ~SHUTTER_MASK;
  PORTC |= (timer1_status == T1STATUS::STOPPED) ? shutter_idle : shutter_running;
}

void reset_OC1A()
{
  TCCR1A = TCCR1A_default & ~bit(WGM10) & ~bit(WGM11);
  TCCR1B = TCCR1B_default & ~bit(WGM12) & ~bit(WGM13);

  // Set the OC1A pin to zero (datasheet section 15.7.3)
  TCCR1C = bit(FOC1A);

  // Restore timer state
  TCCR1A = TCCR1A_default;
  TCCR1B = TCCR1B_default;
}

void start_timer1()
{
  OCR1A = max(timer1_cfg.timer_period_cts >> 4, 1); // set pulse duration 1/8 of period
  ICR1 = max(timer1_cfg.timer_period_cts, 2);       // set timer period

  // Ensure that the delay is at least two counts to avoid overfill
  // Set the timer/counter value and introduce a delay
  TCNT1 = timer1_cfg.timer_period_cts - max(timer1_cfg.cam_delay_cts, 2);

  DDRB |= bit(DDB1); // enable timer output
  GTCCR = 0;         // let it run
}

void stop_timer1()
{
  timer1_status = T1STATUS::STOPPED;
  write_shutter();
  DDRB &= ~bit(DDB1);              // disable timer output
  GTCCR = bit(TSM) | bit(PSRSYNC); // pause the timer
  reset_OC1A();                    // make sure the OC1A pin state is zero
}

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms

  // Enable shutter output on PORTC, pins 0-3
  DDRC |= SHUTTER_MASK;

  // Enable fluidics trigger output on PORTC, pin 4
  DDRC |= bit(DDC4);
  PORTC &= ~bit(PORTC4);

  // Disable camera trigger output on PB1 and set it to zero (timer 1)
  PORTB &= ~bit(PORTB1);
  DDRB &= ~bit(DDB1);

  stop_timer1();
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
      // Write the value to the register with given address
      case 'W':
      case 'w':
        *((uint8_t *)data.reg_addr) = data.reg_value;
        break;

      // Read the value from the given register
      case 'R':
      case 'r':
        Serial.write(*((uint8_t *)data.reg_addr));
        break;

      // Remember the shutter state (keeps only 4 bits)
      case 'S':
      case 's':
        shutter_running = data.shutter_running & SHUTTER_MASK;
        shutter_idle = data.shutter_idle & SHUTTER_MASK;
        write_shutter();
        break;

      // start timer 1
      case 'T':
      case 't':
        if (data.timer_period_cts == 0)
          break; // don't bother, period is zero

        // Read the remaining data from the packet
        charsRead = Serial.readBytes(data.bytes_extra, 6);

        // Create a copy of the packet
        memcpy(&timer1_cfg, &data, sizeof(data));

        // Send trigger to fluidic system
        PORTC |= bit(PORTC4);

        reset_OC1A();

        timer1_status = T1STATUS::SHUTTER_OPENING;
        n_acquired_frames = 0;

        // Start timer 1 - waiting for fludic mixing
        ICR1 = max(timer1_cfg.inj_delay_cts, 2);

        // Enable overflow interrupt (TIMER1_OVF_vect)
        TIMSK1 |= bit(TOIE1);
        interrupts();

        // Rest the timer and let it run
        TCNT1 = 0;
        GTCCR = 0;
        break;

      case 'E':
      case 'e':
        if (data.timer_period_cts == 0)
          break; // don't bother, period is zero

        // change the timer period
        timer1_cfg.timer_period_cts = data.timer_period_cts;
        // no camera delay - immediate change
        timer1_cfg.cam_delay_cts = 0;
        start_timer1();
        break;

      // stop timer 1
      case 'Q':
      case 'q':
        stop_timer1();
        break;
      }
    }
  }
}
