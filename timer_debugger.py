from time import sleep
from serial import Serial
from ctypes import c_uint32
from avrpy.mega328P import Mega328P
from avrpy.avr_bits import *

avr = Mega328P("COM6")


def cu32(value):
    assert value < 2**32, "value exceeds 32 bit"
    return c_uint32(value)


c = Serial("COM8", baudrate=2000000)


def q():
    avr.com.write(b"?xxxx")
    sleep(0.02)
    print(avr.com.read_all().decode())


def p(N=1):
    c.write(cu32(N))
    sleep(1.4e-6 * (N + 100))
    q()


def run():
    avr.TCCR1B &= ~CS12


def pause():
    avr.TCCR1B |= CS12
