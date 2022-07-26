/*******************
DIRECT MEMORY ACCESS
********************/
// convert given memory address to long integer (for pointers),
// then convert it to pointer to unsigned char, and dereference it.
#define MEM_IO_8bit(mem_addr) (*(volatile uint8_t *)(uintptr_t)(mem_addr))

/***************************************
PINOUT AND WIRING DEFINITIONS
***************************************/

// Laser shutters
const uint8_t CY2_PIN = PORT0; // A0
const uint8_t CY3_PIN = PORT1; // A1
const uint8_t CY5_PIN = PORT2; // A2
const uint8_t CY7_PIN = PORT3; // A3
const uint8_t SHUTTERS_MASK = bit(CY2_PIN) | bit(CY3_PIN) | bit(CY5_PIN) | bit(CY7_PIN);
#define SHUTTERS_PORT PORTC
#define SHUTTERS_DDR DDRC

// Fluidic system trigger
const uint8_t FLUIDIC_PIN = PORT2; // pin 2
#define FLUIDIC_PORT PORTD
#define FLUIDIC_DDR DDRD

// Camera trigger
const uint8_t CAMERA_PIN = PORT5; // pin 13
#define CAMERA_PORT PORTB
#define CAMERA_DDR DDRB

/***************************************
COMMUNICATION PROTOCOL DEFINITIONS
***************************************/

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1)    /* set alignment to 1 byte boundary */

// Address and value of register for read/write operations
typedef struct
{
    uint8_t addr;
    uint8_t value;
} Register;

// Fluidics delay
typedef struct g_fluidics
{
    uint16_t fluidics_delay_ms;
} Fluidics;
Fluidics g_fluidics{0};

// Laser shutter states - in active and idle mode
typedef struct
{
    uint8_t idle;
    uint8_t active;
    bool ALEX;
} Shutter;
Shutter g_shutter{0, SHUTTERS_MASK, false};

// Timer 1 configuration for generation of pulsetrains
typedef struct
{
    uint16_t exp_time_n64us;        // laser exposure, in timer counts
    uint16_t n_frames;              // number of frames
    uint16_t interframe_time_n64us; // interframe delay for strobe mode, in timer counts
    uint16_t timelapse_delay_s;     // timelapse delay, in seconds
} Timer1;
Timer1 g_timer1{600, 7, 160, 2};

// Data packet for serial communication
union Data
{
    struct
    {
        uint8_t cmd;

        // All members below share the same 9 bytes of memory
        // Each represents arguments of the command after which it is named
        union
        {
            Register R; // register access (R/W)
            Fluidics F; // fluidics control
            Shutter L;  // laser shutter control and ALEX
            Timer1 T;   // timer1 configuration
        };
    };

    uint8_t bytes[9];
} data;

#pragma pack(pop) /* restore original alignment from stack */

size_t charsRead;

/***************************************
SYSTEM STATUS VARIABLES
***************************************/

bool system_is_up = false;

enum STATUS
{
    IDLE = 0,
    NORMAL_FRAME = 1,
    SKIP_FRAME = 2,
    ALEX_FRAME = 3,
};

volatile uint8_t system_status = STATUS::IDLE;
volatile uint16_t n_acquired_frames = 0; // Total number of acquired frames (pulses to camera)
volatile uint16_t skipped_count = 0;     // Number of already skipped frames during timelapse

volatile uint8_t alex_laser_i = 0;    // Current ALEX channel
volatile uint8_t alex_last_laser = 0; // Index of the last ALEX channel