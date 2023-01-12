#include "uart.h"

#define BAUDRATE 9600UL
#define UART_CTS F_CPU/8/BAUDRATE - 1

void init_UART()
{
	// Set baudrate
	UBRR0 = UART_CTS;
	
	// Enable double speed
	
	UCSR0A = bit(U2X0);
	// Enable receiver and transmitter
	UCSR0B = bit(RXEN0) | bit(TXEN0);

	// Set 8bit asynchronous mode
	UCSR0C = bit(UCSZ01) | bit(UCSZ00);
}


void UART_tx(const char data)
{
	while ( !(UCSR0A & bit(UDRE0)) )
		;

	UDR0 = data;
}

void UART_tx(const char *data)
{
	for (uint8_t i = 0; data[i] != 0; i++)
	{
		UART_tx(data[i]);
	}
}

char UART_rx()
{
	while ( !(UCSR0A & bit(RXC0)) ) // FIXME: or timeout!
		;
	return UDR0;
}