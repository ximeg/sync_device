#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>
#include <Arduino.h>

#ifndef _AVR_IOXXX_H_
#include <avr/iom328.h>
#endif

#include "communication.h"

void send_ok();
void send_err();
void send_err(const char *msg);

#endif // HELPERS_H