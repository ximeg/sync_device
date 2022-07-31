#include <Arduino.h>
#include "triggers.h"
#include "sys_globals.h"

// Configure output ports (data direction registers and default values)
void setup_IO_ports()
{
    FLUIDIC_DDR |= bit(FLUIDIC_PIN);
    FLUIDIC_PORT &= ~bit(FLUIDIC_PIN);

    CAMERA_DDR |= bit(CAMERA_PIN);
    CAMERA_PORT &= ~bit(CAMERA_PIN);

    SHUTTERS_DDR |= SHUTTERS_MASK;
    SHUTTERS_PORT &= ~SHUTTERS_MASK;

    EVENT_LOOP_DDR |= bit(EVENT_LOOP_PIN);
}
