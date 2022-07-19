/***************************************
PINOUT AND WIRING DEFINITIONS
***************************************/

// Laser shutters
const uint8_t CY2_PIN = PORT0;
const uint8_t CY3_PIN = PORT1;
const uint8_t CY5_PIN = PORT2;
const uint8_t CY7_PIN = PORT3;
#define SHUTTERS_PORT PORTC
#define SHUTTERS_DDR DDRC

// Fluidic system trigger
const uint8_t FLUIDIC_PIN = PORT4;
#define FLUIDIC_PORT PORTC
#define FLUIDIC_DDR DDRC

// Camera trigger
const uint8_t CAMERA_PIN = PORT5;
#define CAMERA_PORT PORTC
#define CAMERA_DDR DDRC



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
Shutter g_shutter{0xF, 0};

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

enum T1
{
    STOPPED = 0,
    FIRST_FRAME = 1,
    NORMAL_FRAME = 2,
    SKIP_FRAME = 3,
    ALEX_FRAME = 4,
    LAST_FRAME = 5,
};