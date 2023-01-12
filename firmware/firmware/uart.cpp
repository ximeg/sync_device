#include "uart.h"

#define BAUDRATE 9600UL
#define UART_CTS F_CPU/8/BAUDRATE - 1

void init_UART()
{
	// Set baud rate
	UBRR0 = UART_CTS;
	
	// Enable double speed
	UCSR0A = bit(U2X0);
	
	// Enable receiver and transmitter
	UCSR0B = bit(RXEN0) | bit(TXEN0);

	// Set 8bit asynchronous mode
	UCSR0C = bit(UCSZ01) | bit(UCSZ00);
	
	// Start timer 0 (to detect timeouts)
	TCCR0A = 0;
	TCCR0B = bit(CS02) | bit(CS00);  // /1024 prescaler, overflow every 16.384 ms
}


void UART_tx(const char byte)
{
	while ( !bitRead(UCSR0A, UDRE0) )
		;

	UDR0 = byte;
}

void UART_tx(const char *bytearray)
{
	for (uint8_t i = 0; bytearray[i] != 0; i++)
	{
		UART_tx(bytearray[i]);
	}
}

errcode UART_rx(char &byte)
{
	// Reset timeout clock and remove overflow flag
	TCNT0 = 0;
	TIFR0 = bit(TOV0);
	
	while ( !bitRead(UCSR0A, RXC0) ) // Wait for data...
		if (bitRead(TIFR0, TOV0))  // until a timeout is detected
			return ERR_TIMEOUT;

	byte = UDR0;
	return OK;
}

errcode UART_rx(char *bytearray, uint8_t size)
{
	for (uint8_t i = 0; i < size; i++)
		if (UART_rx(bytearray[i]) == ERR_TIMEOUT)
			return ERR_TIMEOUT;

	return OK;
}