
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
volatile bool system_is_up = false;

volatile uint8_t shutter_running = 0xFF & SHUTTER_MASK;
volatile uint8_t shutter_idle = 0;

volatile uint8_t timer1_status = T1STATUS::STOPPED;
volatile uint16_t timer_period_cts = 0;

volatile uint16_t n_acquired_frames = 0;