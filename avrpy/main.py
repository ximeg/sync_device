from enum import IntEnum
import serial


ms = 0.001
kbps = 1000
Mbps = 1000*kbps

class R(IntEnum):
    OCR1AL = 0x88
    OCR1AH = 0x89
    OCR1BL = 0x8A
    OCR1BH = 0x8B

class AVR(object):
    def __init__(self, port="COM6", baudrate = 2*Mbps, timeout=10*ms):
        self.serial = serial.Serial(port, baudrate=baudrate, timeout=timeout)
        if not self.serial.is_open:
            self.serial.open()
    
    def __del__(self):
        self.serial.close()

    def set_register(self, addr, value):
        self.serial.write(f"W{addr:02x}{value:02x}\n".encode())

    def get_register(self, addr):
        self.serial.flush()
        self.serial.write(f"R{addr:02x}\n".encode())
        val = self.serial.read(2)
        return int(val, 16)

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
        return self.get_16bit_register(R.OCR1AL)

    @OCR1A.setter
    def OCR1A(self, value):
        self.set_16bit_register(R.OCR1AL, value)

    @property
    def OCR1B(self):
        return self.get_16bit_register(R.OCR1BL)

    @OCR1B.setter
    def OCR1B(self, value):
        self.set_16bit_register(R.OCR1BL, value)
