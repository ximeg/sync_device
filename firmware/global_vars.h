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
const uint8_t CY2_PIN = PORT0;
const uint8_t CY3_PIN = PORT2;
const uint8_t CY5_PIN = PORT5;
const uint8_t CY7_PIN = PORT3;
const uint8_t SHUTTERS_MASK = bit(CY2_PIN) | bit(CY3_PIN) | bit(CY5_PIN) | bit(CY7_PIN);
#define SHUTTERS_PORT PORTC
#define SHUTTERS_DDR DDRC

// Fluidic system trigger
const uint8_t FLUIDIC_PIN = PORT2;
#define FLUIDIC_PORT PORTD
#define FLUIDIC_DDR DDRD

// Camera trigger
const uint8_t CAMERA_PIN = PORT5;
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

// Laser shutter states - in active and idle mode
typedef struct
{
    uint8_t active;
    uint8_t idle;
} Shutter;
Shutter g_shutter{SHUTTERS_MASK, 0};

// Timer 1 configuration for generation of pulsetrains
typedef struct
{
    uint16_t timer_period_cts;
    uint16_t n_frames;            // number of frames
    uint16_t shutter_delay_cts;   // camera delay, in timer counts
    uint16_t injection_delay_cts; // injection delay, in timer counts
} Timer1;
Timer1 g_timer1{600, 7, 16, 48};

// Timelapse configuration - number frames to skip
typedef struct
{
    uint16_t skip;
} Timelapse;
Timelapse g_timelapse{0};

// Alternating laser excitation - bit mask showing which spectral channels are active
typedef struct
{
    uint8_t mask;
} ALEX;
ALEX g_ALEX{0};

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
            Register R;  // register access (R/W)
            Shutter S;   // shutter control
            Timer1 T;    // timer configuration and E - exposure time
            Timelapse L; // L - timelapse
            ALEX A;      // A - ALEX mask for shutters
        };
    };

    uint8_t bytes[9];
} data;

#pragma pack(pop) /* restore original alignment from stack */

int charsRead;

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