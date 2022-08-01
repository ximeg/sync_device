#ifndef UART_H
#define UART_H

#include "sys_globals.h"

#define DATA_PACKET_LENGTH 5 // Each data packet has to have this #bytes

void setup_UART();
void check_UART_inbox();

#endif // UART_H