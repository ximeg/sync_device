import serial
from registers import *
from time import sleep
from constants import *

class AVR_Base(object):
    def __init__(self, port="COM6", baudrate = 2*MHz):
        self.serial = serial.Serial(port, baudrate=baudrate)
        if not self.serial.is_open:
            self.serial.open()

        # Opening of the serial port resets Arduino
        # We are going to wait for it to start up
        self.serial.timeout = 5  # seconds
        msg = self.serial.readline().strip().decode()
        self.serial.timeout = 10*ms
        assert msg == "Arduino is ready!", f"Could not connect to Arduino, received message: '{msg}'"
        self.serial.flush()
        sleep(0.01)
    
    def __del__(self):
        self.serial.close()

    def get_register(self, register):
        if register in R8:
            return self._get_8bit_register(register.value)
        elif register in R16:
            return self._get_16bit_register(register.value)
        else:
            ValueError(f"Unknown register {register}") 

    def set_register(self, register, x):
        if register in R8:
            self._set_8bit_register(register.value, x)
        elif register in R16:
            self._set_16bit_register(register.value, x)
        else:
            ValueError(f"Unknown register {register}")

    def _get_8bit_register(self, addr):
        self.serial.flush()
        self.serial.write(bytearray([ord("R"), addr, 0]))
        return ord(self.serial.read(1))

    def _get_16bit_register(self, addrL):
        byte_L = self._get_8bit_register(addrL)
        byte_H = self._get_8bit_register(addrL + 1)
        return byte_H << 8 | byte_L

    def _set_8bit_register(self, addr, x):
        self.serial.write(bytearray([ord("W"), addr, x]))

    def _set_16bit_register(self, addrL, x):
        byte_L = x & 0xff
        byte_H = (x >> 8)
        self._set_8bit_register(addrL + 1, byte_H)
        self._set_8bit_register(addrL, byte_L)

