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

