import serial
from registers import *


ms = 0.001
kbps = 1000
Mbps = 1000*kbps


class AVR(object):
    def __init__(self, port="COM6", baudrate = 2*Mbps, timeout=5*ms):
        self.serial = serial.Serial(port, baudrate=baudrate, timeout=timeout)
        if not self.serial.is_open:
            self.serial.open()
    
    def __del__(self):
        self.serial.close()

    def debug(self, addr, value):
        frame = bytearray([ord("D"), addr, value])
        self.serial.write(frame)
        print(frame)
        return self.serial.readline()

    def set_register(self, addr, value):
        self.serial.write(bytearray([ord("W"), addr, value]))

    def get_register(self, addr):
        self.serial.flush()
        self.serial.write(bytearray([ord("R"), addr, 0]))
        return ord(self.serial.read(1))

    def get_16bit_register(self, addrL):
        byte_L = self.get_register(addrL)
        byte_H = self.get_register(addrL + 1)
        return byte_H << 8 | byte_L

    def set_16bit_register(self, addrL, value):
        byte_L = value & 0xff
        byte_H = (value >> 8)
        self.set_register(addrL + 1, byte_H)
        self.set_register(addrL, byte_L)


    @property
    def OCR1A(self):
        return self.get_16bit_register(R16.OCR1A)
    @OCR1A.setter
    def OCR1A(self, value):
        self.set_16bit_register(R16.OCR1A, value)

    @property
    def OCR1B(self):
        return self.get_16bit_register(R16.OCR1B)
    @OCR1B.setter
    def OCR1B(self, value):
        self.set_16bit_register(R16.OCR1B, value)

    @property
    def TCNT1(self):
        return self.get_16bit_register(R16.TCNT1)
    @TCNT1.setter
    def TCNT1(self, value):
        self.set_16bit_register(R16.TCNT1, value)

    @property
    def ICR1(self):
        return self.get_16bit_register(R16.ICR1)
    @ICR1.setter
    def ICR1(self, value):
        self.set_16bit_register(R16.ICR1, value)

    @property
    def ADC(self):
        return self.get_16bit_register(R16.ADC)
    @ADC.setter
    def ADC(self, value):
        self.set_16bit_register(R16.ADC, value)


    @property
    def TCCR1A(self):
        return self.get_register(R8.TCCR1A)
    @TCCR1A.setter
    def TCCR1A(self, value):
        self.set_register(R8.TCCR1A, value)

    @property
    def TCCR1B(self):
        return self.get_register(R8.TCCR1B)
    @TCCR1B.setter
    def TCCR1B(self, value):
        self.set_register(R8.TCCR1B, value)

    @property
    def TCCR1C(self):
        return self.get_register(R8.TCCR1C)
    @TCCR1C.setter
    def TCCR1C(self, value):
        self.set_register(R8.TCCR1C, value)
