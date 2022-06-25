"""
ATmega328P registers, adapted from
https://github.com/DarkSector/AVR/blob/master/asm/include/m328Pdef.inc
"""

from enum import IntEnum

# 16 bit registers
class R16(IntEnum):
    UBRR0   = 0xc4
    OCR1B   = 0x8a
    OCR1A   = 0x88
    ICR1    = 0x86
    TCNT1   = 0x84
    ADC     = 0x78
    OSCCA   = 0x66
    SP      = 0x5d
    EEAR    = 0x41

# 8 bit (and 16 bit) registers
class R8(IntEnum):
    UDR0    = 0xc6
    UBRR0L  = 0xc4
    UBRR0H  = 0xc5
    UCSR0C  = 0xc2
    UCSR0B  = 0xc1
    UCSR0A  = 0xc0
    TWAMR   = 0xbd
    TWCR    = 0xbc
    TWDR    = 0xbb
    TWAR    = 0xba
    TWSR    = 0xb9
    TWBR    = 0xb8
    ASSR    = 0xb6
    OCR2B   = 0xb4
    OCR2A   = 0xb3
    TCNT2   = 0xb2
    TCCR2B  = 0xb1
    TCCR2A  = 0xb0
    OCR1BL  = 0x8a
    OCR1BH  = 0x8b
    OCR1AL  = 0x88
    OCR1AH  = 0x89
    ICR1L   = 0x86
    ICR1H   = 0x87
    TCNT1L  = 0x84
    TCNT1H  = 0x85
    TCCR1C  = 0x82
    TCCR1B  = 0x81
    TCCR1A  = 0x80
    DIDR1   = 0x7f
    DIDR0   = 0x7e
    ADMUX   = 0x7c
    ADCSRB  = 0x7b
    ADCSRA  = 0x7a
    ADCH    = 0x79
    ADCL    = 0x78
    TIMSK2  = 0x70
    TIMSK1  = 0x6f
    TIMSK0  = 0x6e
    PCMSK1  = 0x6c
    PCMSK2  = 0x6d
    PCMSK0  = 0x6b
    EICRA   = 0x69
    PCICR   = 0x68
    OSCCAL  = 0x66
    PRR     = 0x64
    CLKPR   = 0x61
    WDTCSR  = 0x60
    SREG    = 0x5f
    SPL     = 0x5d
    SPH     = 0x5e
    SPMCSR  = 0x57
    MCUCR   = 0x55
    MCUSR   = 0x54
    SMCR    = 0x53
    ACSR    = 0x50
    SPDR    = 0x4e
    SPSR    = 0x4d
    SPCR    = 0x4c
    GPIOR2  = 0x4b
    GPIOR1  = 0x4a
    OCR0B   = 0x48
    OCR0A   = 0x47
    TCNT0   = 0x46
    TCCR0B  = 0x45
    TCCR0A  = 0x44
    GTCCR   = 0x43
    EEARH   = 0x42
    EEARL   = 0x41
    EEDR    = 0x40
    EECR    = 0x3f
    GPIOR0  = 0x3e
    EIMSK   = 0x3d
    EIFR    = 0x3c
    PCIFR   = 0x3b
    TIFR2   = 0x37
    TIFR1   = 0x36
    TIFR0   = 0x35
    PORTD   = 0x2b
    DDRD    = 0x2a
    PIND    = 0x29
    PORTC   = 0x28
    DDRC    = 0x27
    PINC    = 0x26
    PORTB   = 0x25
    DDRB    = 0x24
    PINB    = 0x23


######################################
##### Bit definitions ################
######################################


# ***** USART0 ***********************
# UDR0 - USART I/O Data Register
UDR0_0  = 1 << 0 # USART I/O Data Register bit 0
UDR0_1  = 1 << 1 # USART I/O Data Register bit 1
UDR0_2  = 1 << 2 # USART I/O Data Register bit 2
UDR0_3  = 1 << 3 # USART I/O Data Register bit 3
UDR0_4  = 1 << 4 # USART I/O Data Register bit 4
UDR0_5  = 1 << 5 # USART I/O Data Register bit 5
UDR0_6  = 1 << 6 # USART I/O Data Register bit 6
UDR0_7  = 1 << 7 # USART I/O Data Register bit 7

# UCSR0A - USART Control and Status Register A
MPCM0   = 1 << 0 # Multi-processor Communication Mode
U2X0    = 1 << 1 # Double the USART transmission speed
UPE0    = 1 << 2 # Parity Error
DOR0    = 1 << 3 # Data overRun
FE0 = 1 << 4 # Framing Error
UDRE0   = 1 << 5 # USART Data Register Empty
TXC0    = 1 << 6 # USART Transmitt Complete
RXC0    = 1 << 7 # USART Receive Complete

# UCSR0B - USART Control and Status Register B
TXB80   = 1 << 0 # Transmit Data Bit 8
RXB80   = 1 << 1 # Receive Data Bit 8
UCSZ02  = 1 << 2 # Character Size
TXEN0   = 1 << 3 # Transmitter Enable
RXEN0   = 1 << 4 # Receiver Enable
UDRIE0  = 1 << 5 # USART Data register Empty Interrupt Enable
TXCIE0  = 1 << 6 # TX Complete Interrupt Enable
RXCIE0  = 1 << 7 # RX Complete Interrupt Enable

# UCSR0C - USART Control and Status Register C
UCPOL0  = 1 << 0 # Clock Polarity
UCSZ00  = 1 << 1 # Character Size
UCPHA0  = 1 << UCSZ00    # For compatibility
UCSZ01  = 1 << 2 # Character Size
UDORD0  = 1 << UCSZ01    # For compatibility
USBS0   = 1 << 3 # Stop Bit Select
UPM00   = 1 << 4 # Parity Mode Bit 0
UPM01   = 1 << 5 # Parity Mode Bit 1
UMSEL00 = 1 << 6 # USART Mode Select
UMSEL0  = 1 << UMSEL00   # For compatibility
UMSEL01 = 1 << 7 # USART Mode Select
UMSEL1  = 1 << UMSEL01   # For compatibility

# UBRR0H - USART Baud Rate Register High Byte
UBRR8   = 1 << 0 # USART Baud Rate Register bit 8
UBRR9   = 1 << 1 # USART Baud Rate Register bit 9
UBRR10  = 1 << 2 # USART Baud Rate Register bit 10
UBRR11  = 1 << 3 # USART Baud Rate Register bit 11

# UBRR0L - USART Baud Rate Register Low Byte
_UBRR0  = 1 << 0 # USART Baud Rate Register bit 0
_UBRR1  = 1 << 1 # USART Baud Rate Register bit 1
UBRR2   = 1 << 2 # USART Baud Rate Register bit 2
UBRR3   = 1 << 3 # USART Baud Rate Register bit 3
UBRR4   = 1 << 4 # USART Baud Rate Register bit 4
UBRR5   = 1 << 5 # USART Baud Rate Register bit 5
UBRR6   = 1 << 6 # USART Baud Rate Register bit 6
UBRR7   = 1 << 7 # USART Baud Rate Register bit 7


# ***** TWI **************************
# TWAMR - TWI (Slave) Address Mask Register
TWAM0   = 1 << 1 # 
TWAMR0  = 1 << TWAM0 # For compatibility
TWAM1   = 1 << 2 # 
TWAMR1  = 1 << TWAM1 # For compatibility
TWAM2   = 1 << 3 # 
TWAMR2  = 1 << TWAM2 # For compatibility
TWAM3   = 1 << 4 # 
TWAMR3  = 1 << TWAM3 # For compatibility
TWAM4   = 1 << 5 # 
TWAMR4  = 1 << TWAM4 # For compatibility
TWAM5   = 1 << 6 # 
TWAMR5  = 1 << TWAM5 # For compatibility
TWAM6   = 1 << 7 # 
TWAMR6  = 1 << TWAM6 # For compatibility

# TWBR - TWI Bit Rate register
TWBR0   = 1 << 0 # 
TWBR1   = 1 << 1 # 
TWBR2   = 1 << 2 # 
TWBR3   = 1 << 3 # 
TWBR4   = 1 << 4 # 
TWBR5   = 1 << 5 # 
TWBR6   = 1 << 6 # 
TWBR7   = 1 << 7 # 

# TWCR - TWI Control Register
TWIE    = 1 << 0 # TWI Interrupt Enable
TWEN    = 1 << 2 # TWI Enable Bit
TWWC    = 1 << 3 # TWI Write Collition Flag
TWSTO   = 1 << 4 # TWI Stop Condition Bit
TWSTA   = 1 << 5 # TWI Start Condition Bit
TWEA    = 1 << 6 # TWI Enable Acknowledge Bit
TWINT   = 1 << 7 # TWI Interrupt Flag

# TWSR - TWI Status Register
TWPS0   = 1 << 0 # TWI Prescaler
TWPS1   = 1 << 1 # TWI Prescaler
TWS3    = 1 << 3 # TWI Status
TWS4    = 1 << 4 # TWI Status
TWS5    = 1 << 5 # TWI Status
TWS6    = 1 << 6 # TWI Status
TWS7    = 1 << 7 # TWI Status

# TWDR - TWI Data register
TWD0    = 1 << 0 # TWI Data Register Bit 0
TWD1    = 1 << 1 # TWI Data Register Bit 1
TWD2    = 1 << 2 # TWI Data Register Bit 2
TWD3    = 1 << 3 # TWI Data Register Bit 3
TWD4    = 1 << 4 # TWI Data Register Bit 4
TWD5    = 1 << 5 # TWI Data Register Bit 5
TWD6    = 1 << 6 # TWI Data Register Bit 6
TWD7    = 1 << 7 # TWI Data Register Bit 7

# TWAR - TWI (Slave) Address register
TWGCE   = 1 << 0 # TWI General Call Recognition Enable Bit
TWA0    = 1 << 1 # TWI (Slave) Address register Bit 0
TWA1    = 1 << 2 # TWI (Slave) Address register Bit 1
TWA2    = 1 << 3 # TWI (Slave) Address register Bit 2
TWA3    = 1 << 4 # TWI (Slave) Address register Bit 3
TWA4    = 1 << 5 # TWI (Slave) Address register Bit 4
TWA5    = 1 << 6 # TWI (Slave) Address register Bit 5
TWA6    = 1 << 7 # TWI (Slave) Address register Bit 6


# ***** TIMER_COUNTER_1 **************
# TIMSK1 - Timer/Counter Interrupt Mask Register
TOIE1   = 1 << 0 # Timer/Counter1 Overflow Interrupt Enable
OCIE1A  = 1 << 1 # Timer/Counter1 Output CompareA Match Interrupt Enable
OCIE1B  = 1 << 2 # Timer/Counter1 Output CompareB Match Interrupt Enable
ICIE1   = 1 << 5 # Timer/Counter1 Input Capture Interrupt Enable

# TIFR1 - Timer/Counter Interrupt Flag register
TOV1    = 1 << 0 # Timer/Counter1 Overflow Flag
OCF1A   = 1 << 1 # Output Compare Flag 1A
OCF1B   = 1 << 2 # Output Compare Flag 1B
ICF1    = 1 << 5 # Input Capture Flag 1

# TCCR1A - Timer/Counter1 Control Register A
WGM10   = 1 << 0 # Waveform Generation Mode
WGM11   = 1 << 1 # Waveform Generation Mode
COM1B0  = 1 << 4 # Compare Output Mode 1B, bit 0
COM1B1  = 1 << 5 # Compare Output Mode 1B, bit 1
COM1A0  = 1 << 6 # Comparet Ouput Mode 1A, bit 0
COM1A1  = 1 << 7 # Compare Output Mode 1A, bit 1

# TCCR1B - Timer/Counter1 Control Register B
CS10    = 1 << 0 # Prescaler source of Timer/Counter 1
CS11    = 1 << 1 # Prescaler source of Timer/Counter 1
CS12    = 1 << 2 # Prescaler source of Timer/Counter 1
WGM12   = 1 << 3 # Waveform Generation Mode
WGM13   = 1 << 4 # Waveform Generation Mode
ICES1   = 1 << 6 # Input Capture 1 Edge Select
ICNC1   = 1 << 7 # Input Capture 1 Noise Canceler

# TCCR1C - Timer/Counter1 Control Register C
FOC1B   = 1 << 6 # 
FOC1A   = 1 << 7 # 

# GTCCR - General Timer/Counter Control Register
PSRSYNC = 1 << 0 # Prescaler Reset Timer/Counter1 and Timer/Counter0
TSM = 1 << 7 # Timer/Counter Synchronization Mode


# ***** TIMER_COUNTER_2 **************
# TIMSK2 - Timer/Counter Interrupt Mask register
TOIE2   = 1 << 0 # Timer/Counter2 Overflow Interrupt Enable
TOIE2A  = 1 << TOIE2 # For compatibility
OCIE2A  = 1 << 1 # Timer/Counter2 Output Compare Match A Interrupt Enable
OCIE2B  = 1 << 2 # Timer/Counter2 Output Compare Match B Interrupt Enable

# TIFR2 - Timer/Counter Interrupt Flag Register
TOV2    = 1 << 0 # Timer/Counter2 Overflow Flag
OCF2A   = 1 << 1 # Output Compare Flag 2A
OCF2B   = 1 << 2 # Output Compare Flag 2B

# TCCR2A - Timer/Counter2 Control Register A
WGM20   = 1 << 0 # Waveform Genration Mode
WGM21   = 1 << 1 # Waveform Genration Mode
COM2B0  = 1 << 4 # Compare Output Mode bit 0
COM2B1  = 1 << 5 # Compare Output Mode bit 1
COM2A0  = 1 << 6 # Compare Output Mode bit 1
COM2A1  = 1 << 7 # Compare Output Mode bit 1

# TCCR2B - Timer/Counter2 Control Register B
CS20    = 1 << 0 # Clock Select bit 0
CS21    = 1 << 1 # Clock Select bit 1
CS22    = 1 << 2 # Clock Select bit 2
WGM22   = 1 << 3 # Waveform Generation Mode
FOC2B   = 1 << 6 # Force Output Compare B
FOC2A   = 1 << 7 # Force Output Compare A

# TCNT2 - Timer/Counter2
TCNT2_0 = 1 << 0 # Timer/Counter 2 bit 0
TCNT2_1 = 1 << 1 # Timer/Counter 2 bit 1
TCNT2_2 = 1 << 2 # Timer/Counter 2 bit 2
TCNT2_3 = 1 << 3 # Timer/Counter 2 bit 3
TCNT2_4 = 1 << 4 # Timer/Counter 2 bit 4
TCNT2_5 = 1 << 5 # Timer/Counter 2 bit 5
TCNT2_6 = 1 << 6 # Timer/Counter 2 bit 6
TCNT2_7 = 1 << 7 # Timer/Counter 2 bit 7

# OCR2A - Timer/Counter2 Output Compare Register A
OCR2A_0 = 1 << 0 # Timer/Counter2 Output Compare Register Bit 0
OCR2A_1 = 1 << 1 # Timer/Counter2 Output Compare Register Bit 1
OCR2A_2 = 1 << 2 # Timer/Counter2 Output Compare Register Bit 2
OCR2A_3 = 1 << 3 # Timer/Counter2 Output Compare Register Bit 3
OCR2A_4 = 1 << 4 # Timer/Counter2 Output Compare Register Bit 4
OCR2A_5 = 1 << 5 # Timer/Counter2 Output Compare Register Bit 5
OCR2A_6 = 1 << 6 # Timer/Counter2 Output Compare Register Bit 6
OCR2A_7 = 1 << 7 # Timer/Counter2 Output Compare Register Bit 7

# OCR2B - Timer/Counter2 Output Compare Register B
OCR2B_0 = 1 << 0 # Timer/Counter2 Output Compare Register Bit 0
OCR2B_1 = 1 << 1 # Timer/Counter2 Output Compare Register Bit 1
OCR2B_2 = 1 << 2 # Timer/Counter2 Output Compare Register Bit 2
OCR2B_3 = 1 << 3 # Timer/Counter2 Output Compare Register Bit 3
OCR2B_4 = 1 << 4 # Timer/Counter2 Output Compare Register Bit 4
OCR2B_5 = 1 << 5 # Timer/Counter2 Output Compare Register Bit 5
OCR2B_6 = 1 << 6 # Timer/Counter2 Output Compare Register Bit 6
OCR2B_7 = 1 << 7 # Timer/Counter2 Output Compare Register Bit 7

# ASSR - Asynchronous Status Register
TCR2BUB = 1 << 0 # Timer/Counter Control Register2 Update Busy
TCR2AUB = 1 << 1 # Timer/Counter Control Register2 Update Busy
OCR2BUB = 1 << 2 # Output Compare Register 2 Update Busy
OCR2AUB = 1 << 3 # Output Compare Register2 Update Busy
TCN2UB  = 1 << 4 # Timer/Counter2 Update Busy
AS2 = 1 << 5 # Asynchronous Timer/Counter2
EXCLK   = 1 << 6 # Enable External Clock Input

# GTCCR - General Timer Counter Control register
PSRASY  = 1 << 1 # Prescaler Reset Timer/Counter2
PSR2    = 1 << PSRASY    # For compatibility
TSM = 1 << 7 # Timer/Counter Synchronization Mode


# ***** AD_CONVERTER *****************
# ADMUX - The ADC multiplexer Selection Register
MUX0    = 1 << 0 # Analog Channel and Gain Selection Bits
MUX1    = 1 << 1 # Analog Channel and Gain Selection Bits
MUX2    = 1 << 2 # Analog Channel and Gain Selection Bits
MUX3    = 1 << 3 # Analog Channel and Gain Selection Bits
ADLAR   = 1 << 5 # Left Adjust Result
REFS0   = 1 << 6 # Reference Selection Bit 0
REFS1   = 1 << 7 # Reference Selection Bit 1

# ADCSRA - The ADC Control and Status register A
ADPS0   = 1 << 0 # ADC  Prescaler Select Bits
ADPS1   = 1 << 1 # ADC  Prescaler Select Bits
ADPS2   = 1 << 2 # ADC  Prescaler Select Bits
ADIE    = 1 << 3 # ADC Interrupt Enable
ADIF    = 1 << 4 # ADC Interrupt Flag
ADATE   = 1 << 5 # ADC  Auto Trigger Enable
ADSC    = 1 << 6 # ADC Start Conversion
ADEN    = 1 << 7 # ADC Enable

# ADCSRB - The ADC Control and Status register B
ADTS0   = 1 << 0 # ADC Auto Trigger Source bit 0
ADTS1   = 1 << 1 # ADC Auto Trigger Source bit 1
ADTS2   = 1 << 2 # ADC Auto Trigger Source bit 2
ACME    = 1 << 6 # 

# ADCH - ADC Data Register High Byte
ADCH0   = 1 << 0 # ADC Data Register High Byte Bit 0
ADCH1   = 1 << 1 # ADC Data Register High Byte Bit 1
ADCH2   = 1 << 2 # ADC Data Register High Byte Bit 2
ADCH3   = 1 << 3 # ADC Data Register High Byte Bit 3
ADCH4   = 1 << 4 # ADC Data Register High Byte Bit 4
ADCH5   = 1 << 5 # ADC Data Register High Byte Bit 5
ADCH6   = 1 << 6 # ADC Data Register High Byte Bit 6
ADCH7   = 1 << 7 # ADC Data Register High Byte Bit 7

# ADCL - ADC Data Register Low Byte
ADCL0   = 1 << 0 # ADC Data Register Low Byte Bit 0
ADCL1   = 1 << 1 # ADC Data Register Low Byte Bit 1
ADCL2   = 1 << 2 # ADC Data Register Low Byte Bit 2
ADCL3   = 1 << 3 # ADC Data Register Low Byte Bit 3
ADCL4   = 1 << 4 # ADC Data Register Low Byte Bit 4
ADCL5   = 1 << 5 # ADC Data Register Low Byte Bit 5
ADCL6   = 1 << 6 # ADC Data Register Low Byte Bit 6
ADCL7   = 1 << 7 # ADC Data Register Low Byte Bit 7

# DIDR0 - Digital Input Disable Register
ADC0D   = 1 << 0 # 
ADC1D   = 1 << 1 # 
ADC2D   = 1 << 2 # 
ADC3D   = 1 << 3 # 
ADC4D   = 1 << 4 # 
ADC5D   = 1 << 5 # 


# ***** ANALOG_COMPARATOR ************
# ACSR - Analog Comparator Control And Status Register
ACIS0   = 1 << 0 # Analog Comparator Interrupt Mode Select bit 0
ACIS1   = 1 << 1 # Analog Comparator Interrupt Mode Select bit 1
ACIC    = 1 << 2 # Analog Comparator Input Capture Enable
ACIE    = 1 << 3 # Analog Comparator Interrupt Enable
ACI = 1 << 4 # Analog Comparator Interrupt Flag
ACO = 1 << 5 # Analog Compare Output
ACBG    = 1 << 6 # Analog Comparator Bandgap Select
ACD = 1 << 7 # Analog Comparator Disable

# DIDR1 - Digital Input Disable Register 1
AIN0D   = 1 << 0 # AIN0 Digital Input Disable
AIN1D   = 1 << 1 # AIN1 Digital Input Disable


# ***** PORTB ************************
# PORTB - Port B Data Register
PORTB0  = 1 << 0 # Port B Data Register bit 0
PB0 = 1 << 0 # For compatibility
PORTB1  = 1 << 1 # Port B Data Register bit 1
PB1 = 1 << 1 # For compatibility
PORTB2  = 1 << 2 # Port B Data Register bit 2
PB2 = 1 << 2 # For compatibility
PORTB3  = 1 << 3 # Port B Data Register bit 3
PB3 = 1 << 3 # For compatibility
PORTB4  = 1 << 4 # Port B Data Register bit 4
PB4 = 1 << 4 # For compatibility
PORTB5  = 1 << 5 # Port B Data Register bit 5
PB5 = 1 << 5 # For compatibility
PORTB6  = 1 << 6 # Port B Data Register bit 6
PB6 = 1 << 6 # For compatibility
PORTB7  = 1 << 7 # Port B Data Register bit 7
PB7 = 1 << 7 # For compatibility

# DDRB - Port B Data Direction Register
DDB0    = 1 << 0 # Port B Data Direction Register bit 0
DDB1    = 1 << 1 # Port B Data Direction Register bit 1
DDB2    = 1 << 2 # Port B Data Direction Register bit 2
DDB3    = 1 << 3 # Port B Data Direction Register bit 3
DDB4    = 1 << 4 # Port B Data Direction Register bit 4
DDB5    = 1 << 5 # Port B Data Direction Register bit 5
DDB6    = 1 << 6 # Port B Data Direction Register bit 6
DDB7    = 1 << 7 # Port B Data Direction Register bit 7

# PINB - Port B Input Pins
PINB0   = 1 << 0 # Port B Input Pins bit 0
PINB1   = 1 << 1 # Port B Input Pins bit 1
PINB2   = 1 << 2 # Port B Input Pins bit 2
PINB3   = 1 << 3 # Port B Input Pins bit 3
PINB4   = 1 << 4 # Port B Input Pins bit 4
PINB5   = 1 << 5 # Port B Input Pins bit 5
PINB6   = 1 << 6 # Port B Input Pins bit 6
PINB7   = 1 << 7 # Port B Input Pins bit 7


# ***** PORTC ************************
# PORTC - Port C Data Register
PORTC0  = 1 << 0 # Port C Data Register bit 0
PC0 = 1 << 0 # For compatibility
PORTC1  = 1 << 1 # Port C Data Register bit 1
PC1 = 1 << 1 # For compatibility
PORTC2  = 1 << 2 # Port C Data Register bit 2
PC2 = 1 << 2 # For compatibility
PORTC3  = 1 << 3 # Port C Data Register bit 3
PC3 = 1 << 3 # For compatibility
PORTC4  = 1 << 4 # Port C Data Register bit 4
PC4 = 1 << 4 # For compatibility
PORTC5  = 1 << 5 # Port C Data Register bit 5
PC5 = 1 << 5 # For compatibility
PORTC6  = 1 << 6 # Port C Data Register bit 6
PC6 = 1 << 6 # For compatibility

# DDRC - Port C Data Direction Register
DDC0    = 1 << 0 # Port C Data Direction Register bit 0
DDC1    = 1 << 1 # Port C Data Direction Register bit 1
DDC2    = 1 << 2 # Port C Data Direction Register bit 2
DDC3    = 1 << 3 # Port C Data Direction Register bit 3
DDC4    = 1 << 4 # Port C Data Direction Register bit 4
DDC5    = 1 << 5 # Port C Data Direction Register bit 5
DDC6    = 1 << 6 # Port C Data Direction Register bit 6

# PINC - Port C Input Pins
PINC0   = 1 << 0 # Port C Input Pins bit 0
PINC1   = 1 << 1 # Port C Input Pins bit 1
PINC2   = 1 << 2 # Port C Input Pins bit 2
PINC3   = 1 << 3 # Port C Input Pins bit 3
PINC4   = 1 << 4 # Port C Input Pins bit 4
PINC5   = 1 << 5 # Port C Input Pins bit 5
PINC6   = 1 << 6 # Port C Input Pins bit 6


# ***** PORTD ************************
# PORTD - Port D Data Register
PORTD0  = 1 << 0 # Port D Data Register bit 0
PD0 = 1 << 0 # For compatibility
PORTD1  = 1 << 1 # Port D Data Register bit 1
PD1 = 1 << 1 # For compatibility
PORTD2  = 1 << 2 # Port D Data Register bit 2
PD2 = 1 << 2 # For compatibility
PORTD3  = 1 << 3 # Port D Data Register bit 3
PD3 = 1 << 3 # For compatibility
PORTD4  = 1 << 4 # Port D Data Register bit 4
PD4 = 1 << 4 # For compatibility
PORTD5  = 1 << 5 # Port D Data Register bit 5
PD5 = 1 << 5 # For compatibility
PORTD6  = 1 << 6 # Port D Data Register bit 6
PD6 = 1 << 6 # For compatibility
PORTD7  = 1 << 7 # Port D Data Register bit 7
PD7 = 1 << 7 # For compatibility

# DDRD - Port D Data Direction Register
DDD0    = 1 << 0 # Port D Data Direction Register bit 0
DDD1    = 1 << 1 # Port D Data Direction Register bit 1
DDD2    = 1 << 2 # Port D Data Direction Register bit 2
DDD3    = 1 << 3 # Port D Data Direction Register bit 3
DDD4    = 1 << 4 # Port D Data Direction Register bit 4
DDD5    = 1 << 5 # Port D Data Direction Register bit 5
DDD6    = 1 << 6 # Port D Data Direction Register bit 6
DDD7    = 1 << 7 # Port D Data Direction Register bit 7

# PIND - Port D Input Pins
PIND0   = 1 << 0 # Port D Input Pins bit 0
PIND1   = 1 << 1 # Port D Input Pins bit 1
PIND2   = 1 << 2 # Port D Input Pins bit 2
PIND3   = 1 << 3 # Port D Input Pins bit 3
PIND4   = 1 << 4 # Port D Input Pins bit 4
PIND5   = 1 << 5 # Port D Input Pins bit 5
PIND6   = 1 << 6 # Port D Input Pins bit 6
PIND7   = 1 << 7 # Port D Input Pins bit 7


# ***** TIMER_COUNTER_0 **************
# TIMSK0 - Timer/Counter0 Interrupt Mask Register
TOIE0   = 1 << 0 # Timer/Counter0 Overflow Interrupt Enable
OCIE0A  = 1 << 1 # Timer/Counter0 Output Compare Match A Interrupt Enable
OCIE0B  = 1 << 2 # Timer/Counter0 Output Compare Match B Interrupt Enable

# TIFR0 - Timer/Counter0 Interrupt Flag register
TOV0    = 1 << 0 # Timer/Counter0 Overflow Flag
OCF0A   = 1 << 1 # Timer/Counter0 Output Compare Flag 0A
OCF0B   = 1 << 2 # Timer/Counter0 Output Compare Flag 0B

# TCCR0A - Timer/Counter  Control Register A
WGM00   = 1 << 0 # Waveform Generation Mode
WGM01   = 1 << 1 # Waveform Generation Mode
COM0B0  = 1 << 4 # Compare Output Mode, Fast PWm
COM0B1  = 1 << 5 # Compare Output Mode, Fast PWm
COM0A0  = 1 << 6 # Compare Output Mode, Phase Correct PWM Mode
COM0A1  = 1 << 7 # Compare Output Mode, Phase Correct PWM Mode

# TCCR0B - Timer/Counter Control Register B
CS00    = 1 << 0 # Clock Select
CS01    = 1 << 1 # Clock Select
CS02    = 1 << 2 # Clock Select
WGM02   = 1 << 3 # 
FOC0B   = 1 << 6 # Force Output Compare B
FOC0A   = 1 << 7 # Force Output Compare A

# TCNT0 - Timer/Counter0
TCNT0_0 = 1 << 0 # 
TCNT0_1 = 1 << 1 # 
TCNT0_2 = 1 << 2 # 
TCNT0_3 = 1 << 3 # 
TCNT0_4 = 1 << 4 # 
TCNT0_5 = 1 << 5 # 
TCNT0_6 = 1 << 6 # 
TCNT0_7 = 1 << 7 # 

# OCR0A - Timer/Counter0 Output Compare Register
OCR0A_0 = 1 << 0 # 
OCR0A_1 = 1 << 1 # 
OCR0A_2 = 1 << 2 # 
OCR0A_3 = 1 << 3 # 
OCR0A_4 = 1 << 4 # 
OCR0A_5 = 1 << 5 # 
OCR0A_6 = 1 << 6 # 
OCR0A_7 = 1 << 7 # 

# OCR0B - Timer/Counter0 Output Compare Register
OCR0B_0 = 1 << 0 # 
OCR0B_1 = 1 << 1 # 
OCR0B_2 = 1 << 2 # 
OCR0B_3 = 1 << 3 # 
OCR0B_4 = 1 << 4 # 
OCR0B_5 = 1 << 5 # 
OCR0B_6 = 1 << 6 # 
OCR0B_7 = 1 << 7 # 

# GTCCR - General Timer/Counter Control Register
PSRSYNC = 1 << 0 # Prescaler Reset Timer/Counter1 and Timer/Counter0
PSR10   = 1 << PSRSYNC   # For compatibility
TSM = 1 << 7 # Timer/Counter Synchronization Mode


# ***** EXTERNAL_INTERRUPT ***********
# EICRA - External Interrupt Control Register
ISC00   = 1 << 0 # External Interrupt Sense Control 0 Bit 0
ISC01   = 1 << 1 # External Interrupt Sense Control 0 Bit 1
ISC10   = 1 << 2 # External Interrupt Sense Control 1 Bit 0
ISC11   = 1 << 3 # External Interrupt Sense Control 1 Bit 1

# EIMSK - External Interrupt Mask Register
INT0    = 1 << 0 # External Interrupt Request 0 Enable
INT1    = 1 << 1 # External Interrupt Request 1 Enable

# EIFR - External Interrupt Flag Register
INTF0   = 1 << 0 # External Interrupt Flag 0
INTF1   = 1 << 1 # External Interrupt Flag 1

# PCICR - Pin Change Interrupt Control Register
PCIE0   = 1 << 0 # Pin Change Interrupt Enable 0
PCIE1   = 1 << 1 # Pin Change Interrupt Enable 1
PCIE2   = 1 << 2 # Pin Change Interrupt Enable 2

# PCMSK2 - Pin Change Mask Register 2
PCINT16 = 1 << 0 # Pin Change Enable Mask 16
PCINT17 = 1 << 1 # Pin Change Enable Mask 17
PCINT18 = 1 << 2 # Pin Change Enable Mask 18
PCINT19 = 1 << 3 # Pin Change Enable Mask 19
PCINT20 = 1 << 4 # Pin Change Enable Mask 20
PCINT21 = 1 << 5 # Pin Change Enable Mask 21
PCINT22 = 1 << 6 # Pin Change Enable Mask 22
PCINT23 = 1 << 7 # Pin Change Enable Mask 23

# PCMSK1 - Pin Change Mask Register 1
PCINT8  = 1 << 0 # Pin Change Enable Mask 8
PCINT9  = 1 << 1 # Pin Change Enable Mask 9
PCINT10 = 1 << 2 # Pin Change Enable Mask 10
PCINT11 = 1 << 3 # Pin Change Enable Mask 11
PCINT12 = 1 << 4 # Pin Change Enable Mask 12
PCINT13 = 1 << 5 # Pin Change Enable Mask 13
PCINT14 = 1 << 6 # Pin Change Enable Mask 14

# PCMSK0 - Pin Change Mask Register 0
PCINT0  = 1 << 0 # Pin Change Enable Mask 0
PCINT1  = 1 << 1 # Pin Change Enable Mask 1
PCINT2  = 1 << 2 # Pin Change Enable Mask 2
PCINT3  = 1 << 3 # Pin Change Enable Mask 3
PCINT4  = 1 << 4 # Pin Change Enable Mask 4
PCINT5  = 1 << 5 # Pin Change Enable Mask 5
PCINT6  = 1 << 6 # Pin Change Enable Mask 6
PCINT7  = 1 << 7 # Pin Change Enable Mask 7

# PCIFR - Pin Change Interrupt Flag Register
PCIF0   = 1 << 0 # Pin Change Interrupt Flag 0
PCIF1   = 1 << 1 # Pin Change Interrupt Flag 1
PCIF2   = 1 << 2 # Pin Change Interrupt Flag 2


# ***** SPI **************************
# SPDR - SPI Data Register
SPDR0   = 1 << 0 # SPI Data Register bit 0
SPDR1   = 1 << 1 # SPI Data Register bit 1
SPDR2   = 1 << 2 # SPI Data Register bit 2
SPDR3   = 1 << 3 # SPI Data Register bit 3
SPDR4   = 1 << 4 # SPI Data Register bit 4
SPDR5   = 1 << 5 # SPI Data Register bit 5
SPDR6   = 1 << 6 # SPI Data Register bit 6
SPDR7   = 1 << 7 # SPI Data Register bit 7

# SPSR - SPI Status Register
SPI2X   = 1 << 0 # Double SPI Speed Bit
WCOL    = 1 << 6 # Write Collision Flag
SPIF    = 1 << 7 # SPI Interrupt Flag

# SPCR - SPI Control Register
SPR0    = 1 << 0 # SPI Clock Rate Select 0
SPR1    = 1 << 1 # SPI Clock Rate Select 1
CPHA    = 1 << 2 # Clock Phase
CPOL    = 1 << 3 # Clock polarity
MSTR    = 1 << 4 # Master/Slave Select
DORD    = 1 << 5 # Data Order
SPE = 1 << 6 # SPI Enable
SPIE    = 1 << 7 # SPI Interrupt Enable


# ***** WATCHDOG *********************
# WDTCSR - Watchdog Timer Control Register
WDP0    = 1 << 0 # Watch Dog Timer Prescaler bit 0
WDP1    = 1 << 1 # Watch Dog Timer Prescaler bit 1
WDP2    = 1 << 2 # Watch Dog Timer Prescaler bit 2
WDE = 1 << 3 # Watch Dog Enable
WDCE    = 1 << 4 # Watchdog Change Enable
WDP3    = 1 << 5 # Watchdog Timer Prescaler Bit 3
WDIE    = 1 << 6 # Watchdog Timeout Interrupt Enable
WDIF    = 1 << 7 # Watchdog Timeout Interrupt Flag


# ***** CPU **************************
# SREG - Status Register
SREG_C  = 1 << 0 # Carry Flag
SREG_Z  = 1 << 1 # Zero Flag
SREG_N  = 1 << 2 # Negative Flag
SREG_V  = 1 << 3 # Two's Complement Overflow Flag
SREG_S  = 1 << 4 # Sign Bit
SREG_H  = 1 << 5 # Half Carry Flag
SREG_T  = 1 << 6 # Bit Copy Storage
SREG_I  = 1 << 7 # Global Interrupt Enable
# OSCCAL - Oscillator Calibration Value
CAL0    = 1 << 0 # Oscillator Calibration Value Bit0
CAL1    = 1 << 1 # Oscillator Calibration Value Bit1
CAL2    = 1 << 2 # Oscillator Calibration Value Bit2
CAL3    = 1 << 3 # Oscillator Calibration Value Bit3
CAL4    = 1 << 4 # Oscillator Calibration Value Bit4
CAL5    = 1 << 5 # Oscillator Calibration Value Bit5
CAL6    = 1 << 6 # Oscillator Calibration Value Bit6
CAL7    = 1 << 7 # Oscillator Calibration Value Bit7
# CLKPR - Clock Prescale Register
CLKPS0  = 1 << 0 # Clock Prescaler Select Bit 0
CLKPS1  = 1 << 1 # Clock Prescaler Select Bit 1
CLKPS2  = 1 << 2 # Clock Prescaler Select Bit 2
CLKPS3  = 1 << 3 # Clock Prescaler Select Bit 3
CLKPCE  = 1 << 7 # Clock Prescaler Change Enable
# SPMCSR - Store Program Memory Control and Status Register
SELFPRGEN   = 1 << 0 # Self Programming Enable
PGERS   = 1 << 1 # Page Erase
PGWRT   = 1 << 2 # Page Write
BLBSET  = 1 << 3 # Boot Lock Bit Set
RWWSRE  = 1 << 4 # Read-While-Write section read enable
RWWSB   = 1 << 6 # Read-While-Write Section Busy
SPMIE   = 1 << 7 # SPM Interrupt Enable
# MCUCR - MCU Control Register
IVCE    = 1 << 0 # 
IVSEL   = 1 << 1 # 
PUD = 1 << 4 # 
BODSE   = 1 << 5 # BOD Sleep Enable
BODS    = 1 << 6 # BOD Sleep
# MCUSR - MCU Status Register
PORF    = 1 << 0 # Power-on reset flag
EXTRF   = 1 << 1 # External Reset Flag
EXTREF  = 1 << EXTRF # For compatibility
BORF    = 1 << 2 # Brown-out Reset Flag
WDRF    = 1 << 3 # Watchdog Reset Flag
# SMCR - Sleep Mode Control Register
SE  = 1 << 0 # Sleep Enable
SM0 = 1 << 1 # Sleep Mode Select Bit 0
SM1 = 1 << 2 # Sleep Mode Select Bit 1
SM2 = 1 << 3 # Sleep Mode Select Bit 2
# GPIOR2 - General Purpose I/O Register 2
GPIOR20 = 1 << 0 # 
GPIOR21 = 1 << 1 # 
GPIOR22 = 1 << 2 # 
GPIOR23 = 1 << 3 # 
GPIOR24 = 1 << 4 # 
GPIOR25 = 1 << 5 # 
GPIOR26 = 1 << 6 # 
GPIOR27 = 1 << 7 # 
# GPIOR1 - General Purpose I/O Register 1
GPIOR10 = 1 << 0 # 
GPIOR11 = 1 << 1 # 
GPIOR12 = 1 << 2 # 
GPIOR13 = 1 << 3 # 
GPIOR14 = 1 << 4 # 
GPIOR15 = 1 << 5 # 
GPIOR16 = 1 << 6 # 
GPIOR17 = 1 << 7 # 
# GPIOR0 - General Purpose I/O Register 0
GPIOR00 = 1 << 0 # 
GPIOR01 = 1 << 1 # 
GPIOR02 = 1 << 2 # 
GPIOR03 = 1 << 3 # 
GPIOR04 = 1 << 4 # 
GPIOR05 = 1 << 5 # 
GPIOR06 = 1 << 6 # 
GPIOR07 = 1 << 7 # 
# PRR - Power Reduction Register
PRADC   = 1 << 0 # Power Reduction ADC
PRUSART0    = 1 << 1 # Power Reduction USART
PRSPI   = 1 << 2 # Power Reduction Serial Peripheral Interface
PRTIM1  = 1 << 3 # Power Reduction Timer/Counter1
PRTIM0  = 1 << 5 # Power Reduction Timer/Counter0
PRTIM2  = 1 << 6 # Power Reduction Timer/Counter2
PRTWI   = 1 << 7 # Power Reduction TWI
# ***** EEPROM ***********************
# EEARL - EEPROM Address Register Low Byte
EEAR0   = 1 << 0 # EEPROM Read/Write Access Bit 0
EEAR1   = 1 << 1 # EEPROM Read/Write Access Bit 1
EEAR2   = 1 << 2 # EEPROM Read/Write Access Bit 2
EEAR3   = 1 << 3 # EEPROM Read/Write Access Bit 3
EEAR4   = 1 << 4 # EEPROM Read/Write Access Bit 4
EEAR5   = 1 << 5 # EEPROM Read/Write Access Bit 5
EEAR6   = 1 << 6 # EEPROM Read/Write Access Bit 6
EEAR7   = 1 << 7 # EEPROM Read/Write Access Bit 7
# EEARH - EEPROM Address Register High Byte
EEAR8   = 1 << 0 # EEPROM Read/Write Access Bit 8
EEAR9   = 1 << 1 # EEPROM Read/Write Access Bit 9
# EEDR - EEPROM Data Register
EEDR0   = 1 << 0 # EEPROM Data Register bit 0
EEDR1   = 1 << 1 # EEPROM Data Register bit 1
EEDR2   = 1 << 2 # EEPROM Data Register bit 2
EEDR3   = 1 << 3 # EEPROM Data Register bit 3
EEDR4   = 1 << 4 # EEPROM Data Register bit 4
EEDR5   = 1 << 5 # EEPROM Data Register bit 5
EEDR6   = 1 << 6 # EEPROM Data Register bit 6
EEDR7   = 1 << 7 # EEPROM Data Register bit 7
# EECR - EEPROM Control Register
EERE    = 1 << 0 # EEPROM Read Enable
EEPE    = 1 << 1 # EEPROM Write Enable
EEMPE   = 1 << 2 # EEPROM Master Write Enable
EERIE   = 1 << 3 # EEPROM Ready Interrupt Enable
EEPM0   = 1 << 4 # EEPROM Programming Mode Bit 0
EEPM1   = 1 << 5 # EEPROM Programming Mode Bit 1
# ***** LOCKSBITS ********************************************************
LB1 = 1 << 0 # Lock bit
LB2 = 1 << 1 # Lock bit
BLB01   = 1 << 2 # Boot Lock bit
BLB02   = 1 << 3 # Boot Lock bit
BLB11   = 1 << 4 # Boot lock bit
BLB12   = 1 << 5 # Boot lock bit
# ***** FUSES ************************************************************
# LOW fuse bits
CKSEL0  = 1 << 0 # Select Clock Source
CKSEL1  = 1 << 1 # Select Clock Source
CKSEL2  = 1 << 2 # Select Clock Source
CKSEL3  = 1 << 3 # Select Clock Source
SUT0    = 1 << 4 # Select start-up time
SUT1    = 1 << 5 # Select start-up time
CKOUT   = 1 << 6 # Clock output
CKDIV8  = 1 << 7 # Divide clock by 8
# HIGH fuse bits
BOOTRST = 1 << 0 # Select reset vector
BOOTSZ0 = 1 << 1 # Select boot size
BOOTSZ1 = 1 << 2 # Select boot size
EESAVE  = 1 << 3 # EEPROM memory is preserved through chip erase
WDTON   = 1 << 4 # Watchdog Timer Always On
SPIEN   = 1 << 5 # Enable Serial programming and Data Downloading
DWEN    = 1 << 6 # debugWIRE Enable
RSTDISBL    = 1 << 7 # External reset disable
# EXTENDED fuse bits
BODLEVEL0   = 1 << 0 # Brown-out Detector trigger level
BODLEVEL1   = 1 << 1 # Brown-out Detector trigger level
BODLEVEL2   = 1 << 2 # Brown-out Detector trigger level
