/**
 * @file uart.h
 * @author Roman Kiselev (roman.kiselev@stjude.org)
 * @brief Implementation of the communication protocol
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include "sys_globals.h"

// Initializes hardware UART module and TCNT0 to keep track of timeouts
void init_UART();

// Send "OK\n" to the host
void UART_tx_ok();

// Send "ERR\n" to the host
void UART_tx_err();

// Send "ERR\n" and an error message to the host
void UART_tx_err(const char *msg);

// Send data to the host
void UART_tx(const char byte);
void UART_tx(const char *bytearray);

// Retrieve data received from host. If no data received within 16ms, return ERR_TIMEOUT
errcode UART_rx(char &byte);
errcode UART_rx(char *bytearray, uint8_t size);

// Retrieve and process a 5-byte data packet from host, if available
void poll_UART();