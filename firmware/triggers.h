#ifndef TRIGGERS_H
#define TRIGGERS_H

#include "sys_globals.h"

extern void setup_IO_ports();

inline void fluidics_pin_up() { FLUIDIC_PORT |= bit(FLUIDIC_PIN); }
inline void fluidics_pin_down() { FLUIDIC_PORT &= ~bit(FLUIDIC_PIN); }

#endif // TRIGGERS_H