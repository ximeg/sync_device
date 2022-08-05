#include "uart.h"
#include "utils.h"
#include "triggers.h"
#include "events.h"
#include "timer1.h"

/********************************
COMMUNICATION PROTOCOL DEFINITION
********************************/

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

// convert given memory address to long integer (for pointers),
// then convert it to pointer to unsigned char, and dereference it.
#define MEM_IO_8bit(mem_addr) (*(volatile uint8_t *)(uintptr_t)(mem_addr))

// UART responses
void send_ok() { Serial.print("OK\n"); }
void send_err() { Serial.print("ERR\n"); }
void send_err(const char *msg)
{
    Serial.print(msg);
    Serial.print("\n");
}

void setup_UART()
{
    Serial.begin(2000000);
    Serial.setTimeout(10); // ms

    // Wait until the serial port is ready
    while (!Serial)
    {
    }
    Serial.flush();

    // Notify the host that we are ready
    Serial.print("Arduino is ready. Firmware version: ");
    Serial.print(VERSION);
}

void parse_UART_command()
{
    switch (data.cmd)
    {
    // Read register
    case 'R':
        Serial.write(MEM_IO_8bit(data.R.addr));
        break;

    // Write register
    case 'W':
        MEM_IO_8bit(data.R.addr) = data.R.value;
        break;

    // Set fluidics delay
    case 'F':
        sys.fluidics_delay_us = data.fluidics_delay_us;
        send_ok();
        break;

    // Set laser shutter and ALEX states
    case 'L':
        if (data.Shutter.ALEX)
        {
            if (count_bits(data.Shutter.active) < 2)
            {
                // Less than two spectral channels - can't do ALEX!
                send_err("ALEX error: not enough channels");
                break;
            }
        }
        sys.shutter_active = decode_shutter_bits(data.Shutter.active);
        sys.shutter_idle = decode_shutter_bits(data.Shutter.idle);
        sys.ALEX_enabled = data.Shutter.ALEX;

        if (sys.status == STATUS::IDLE)
            write_shutters(sys.shutter_idle);

        send_ok();
        break;

    // Set interframe interval
    case 'I':
        if (data.interframe_time_us < 40)
        {
            send_err("ACQ error: interframe interval is too short");
            break;
        }
        sys.interframe_time_us = data.interframe_time_us;
        // setup_timer1(sys.interframe_time_us, sys.strobe_duration_us, sys.strobe_duration_us + (sys.interframe_time_us >> 3));
        send_ok();
        break;

    // Set strobe flash duration
    case 'E':
        sys.strobe_duration_us = data.strobe_duration_us;
        // setup_timer1(sys.interframe_time_us, sys.strobe_duration_us, sys.strobe_duration_us + (sys.interframe_time_us >> 3));
        send_ok();
        break;

    // Set ALEX cycle delay (for timelapse with ALEX)
    case 'A':
        sys.ALEX_cycle_delay_us = data.ALEX_cycle_delay_us;
        send_ok();
        break;

    // Start continuous image acquisition
    case 'C':
        sys.n_frames = data.n_frames;
        //        start_continuous_imaging();
        send_ok();
        break;

    // Stop image acquisition
    case 'Q':
        // stop_acquisition();
        send_ok();
        break;

    default:
        break;
    }
}

void poll_UART()
{
    if (Serial.available())
    {
        if (Serial.readBytes(data.bytes, DATA_PACKET_LENGTH) == DATA_PACKET_LENGTH)
        {
            parse_UART_command();
        }
    }
}

/*

f

    // Start stroboscopic image acquisition
    case 'S':
        if (sys.strobe_duration_us + 12000 > sys.interframe_time_us)
        {
            send_err("Not enough time to readout the sensor. Check strobe and interframe timings");
            break;
        }
        break;


    default:
        break;


}
}
*/