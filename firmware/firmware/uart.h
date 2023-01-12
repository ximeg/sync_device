#pragma once

#include "sys_globals.h"


void init_UART();

void UART_tx(const char data);
char UART_rx();
