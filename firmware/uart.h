#ifndef UART_H
#define UART_H

#define DATA_PACKET_LENGTH 5 // Each data packet has to have this #bytes

extern void setup_UART();
extern void check_UART_inbox();

#endif // UART_H