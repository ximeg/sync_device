#ifndef SYS_GLOBALS_H
#define SYS_GLOBALS_H

#include <stdint.h>
#include <Arduino.h>

#define VERSION "0.3.0\n"

/****************************
PINOUT AND WIRING DEFINITIONS
****************************/
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

/**********************
SYSTEM STATUS VARIABLES
**********************/
static uint32_t t0; // current system time

// System status
enum STATUS
{
    IDLE = 0,             // Doing nothing, waiting for commands
    CONTINUOUS_ACQ_START, // First frame that will be discarded
    CONTINUOUS_ACQ,       // Running continuous acquisition
    CONTINUOUS_ACQ_END,   // Waiting for camera readout at the end of continuous acquisition
    STROBO_ACQ,           // Running stroboscopic acquisition (includes ALEX and timelapse)
};

inline struct SystemSettings
{
    STATUS status;
    uint64_t time;
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

#endif // SYS_GLOBALS_H
