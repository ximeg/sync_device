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

// Event loop monitor pin - changes value after every cycle
const uint8_t EVENT_LOOP_PIN = PORT5; // A5
#define EVENT_LOOP_PIN_FLIP PINC
#define EVENT_LOOP_DDR DDRC

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
    bool ALEX;
} LaserShutter;

// Data packet for serial communication
union Data
{
    struct
    {
        uint8_t cmd;

        // All members below share the same chunk of memory
        union
        {
            Register R;                   // register access (R/W)
            int32_t fluidics_delay_us;    // fluidics injection delay. If negative, happens before imaging
            LaserShutter Shutter;         // laser shutter control and ALEX on/off
            uint32_t interframe_time_us;  // time between frames in any imaging mode
            uint32_t strobe_duration_us;  // duration of laser flash in stroboscopic mode
            uint32_t ALEX_cycle_delay_us; // duration of delay between ALEX cycles
            uint32_t n_frames;            // number of frames to acquire
        };
    };

    uint8_t bytes[9];
} data;

#pragma pack(pop) /* restore original alignment from stack */

/***************************************
SYSTEM STATUS VARIABLES
***************************************/

bool system_is_up = false;

// System status
enum STATUS
{
    IDLE = 0,             // Doing nothing, waiting for commands
    CONTINUOUS_ACQ_START, // First frame that will be discarded
    CONTINUOUS_ACQ,       // Running continuous acquisition
    CONTINUOUS_ACQ_END,   // Waiting for camera readout at the end of continuous acquisition
    STROBO_ACQ,           // Running stroboscopic acquisition (includes ALEX and timelapse)
};

static struct SystemSettings
{
    STATUS status;
    bool up;
    int32_t fluidics_delay_us;
    uint32_t interframe_time_us;
    uint32_t strobe_duration_us;
    uint32_t ALEX_cycle_delay_us;
    bool ALEX_enabled;
    uint8_t ALEX_current_channel;
    uint8_t ALEX_last_channel;
    uint32_t n_frames;
    uint32_t n_acquired_frames; // Total number of acquired frames (pulses to camera)
    uint8_t shutter_active;     // Actual bit pattern for the IO port, see `decode_shutter_bits()`
    uint8_t shutter_idle;

    // Default values
    SystemSettings() : shutter_active(SHUTTERS_MASK),
                       interframe_time_us(100000),
                       strobe_duration_us(25000) {}
} sys;

typedef struct
{
    uint32_t up;   // timestamp of TTL raising edge, in microseconds
    uint32_t down; // timestamp of TTL falling edge, in microseconds
} Trigger;

// This structure holds timestamps of the events. Once we reach the timepoint
// of the event, it should be triggered.
static struct
{
    Trigger camera_TTL;
    Trigger fluidics_TTL;
    Trigger shutter_TTL;
} next_event;
