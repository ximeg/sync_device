#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <Arduino.h>

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
typedef union
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
} Data;

inline Data data;

#pragma pack(pop) /* restore original alignment from stack */

#endif // COMMUNICATION_H