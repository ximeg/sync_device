/**
 * @file uart.h
 * @author Roman Kiselev (roman.kiselev@stjude.org)
 * @brief UART communication module for Arduino sync device. See .cpp file for the communication protocol
 * @version 0.3
 * @date 2022-08-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef UART_H
#define UART_H

#include "sys_globals.h"

// Each data packet has to have this exact number of bytes
#define DATA_PACKET_LENGTH 5

void setup_UART();
void check_UART_inbox();

#endif // UART_H