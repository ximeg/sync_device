"""
ATmega328P registers, adapted from
https://github.com/DarkSector/AVR/blob/master/asm/include/m328Pdef.inc
"""

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
