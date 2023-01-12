#pragma once

#include "sys_globals.h"

void init_UART();

void UART_tx(const char byte);
void UART_tx(const char *bytearray);


errcode UART_rx(char &byte);
errcode UART_rx(char *bytearray, uint8_t size);
