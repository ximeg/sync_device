#include "uart.h"
#include "timers.h"

#define BAUDRATE 2000000UL
#define UART_CTS F_CPU/8/BAUDRATE - 1

// convert given memory address to long integer (for pointers),
// then convert it to pointer to unsigned char, and dereference it.
#define MEM_IO_8bit(mem_addr) (*(volatile uint8_t *)(uintptr_t)(mem_addr))


/********************************
COMMUNICATION PROTOCOL DEFINITION
********************************/

// Address and value of register for read/write operations
typedef struct
{
	uint8_t addr;
	uint8_t value;
} Register;

// Laser shutter states - in active and idle mode
typedef struct
{
	uint8_t active;
	uint8_t idle;
	bool ALEX;
} LaserShutter;

// Data packet for serial communication
union Data
{
	struct
	{
		uint8_t cmd;

		// All members below share the same chunk of memory
		union
		{
			Register R;                   // register access (R/W)
			int32_t int32_value;
			uint32_t uint32_value;
		};
	};

	char bytes[5];
};


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



void parse_UART_command(const Data data)
{
	switch (data.cmd)
	{
		// Read register
		case 'R':
		UART_tx(MEM_IO_8bit(data.R.addr));
		break;

		// Write register
		case 'W':
		MEM_IO_8bit(data.R.addr) = data.R.value;
		break;
	
		// Start acquisition
		case 'S':
		start_acq();
		break;

		default:
		break;
	}
}

void poll_UART()
{
	Data data;
	if (UART_rx(data.bytes, 5) == OK)
	{
		parse_UART_command(data);
	}
}
