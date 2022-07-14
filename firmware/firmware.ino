#define VERSION "0.1.0"

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
PROGRAM LOGIC
************/

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(2000000);
  Serial.setTimeout(10); // ms
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

      case 'R':
      case 'r':
        // Read the value from the given register

      case 'T': // start timer 1
        Serial.readBytes(data.bytes_extra, 6);
        Serial.print(" exp time:");
        Serial.print(data.timer_period_cts);
        Serial.print(" n frames:");
        Serial.print(data.n_frames);
        Serial.print(" camdelay:");
        Serial.print(data.cam_delay_cts);
        Serial.print(" injdelay:");
        Serial.println(data.inj_delay_cts);
        break;
      case 't': // stop timer 1
        Serial.println("Stopping timer 1");
        break;
      }
    }
  }
}
