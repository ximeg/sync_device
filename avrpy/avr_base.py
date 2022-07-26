from cProfile import run
from numpy import byte
import serial
from enum import Enum
from time import sleep
from .__version__ import __version__
from .constants import ms, MHz
import ctypes


class RegisterBase(Enum):
    """
    Base class to create microcontroller-specific list of registers.
    Stores register name, address, and type (8-bit or 16-bit).
    """

    def __repr__(self):
        return f"{self.name}: {self.bits}-bit register at {self.addr}"

    @property
    def bits(self):
        return self.value[1]

    @property
    def addr(self):
        return self.value[0]


def _compare_versions(v1, v2):
    return {
        k: a == b
        for k, a, b in zip(["major", "minor", "patch"], v1.split("."), v2.split("."))
    }


def _pad(data: bytearray, length=9):
    return data + bytearray([0] * (length - len(data)))


class AVR_Base(object):
    """
    Abstract AVR microcontroller unit (MCU). You can read/write registers if you know their address.
    However, it is recommended to derive a class for a specific MCU by calling `define_AVR` with list of registers
    """

    def __init__(self, port, baudrate):
        self.serial = serial.Serial(port, baudrate=baudrate)
        if not self.serial.is_open:
            self.serial.open()

        # Opening of the serial port resets Arduino
        # We are going to wait for it to start up
        self.serial.timeout = 5  # seconds
        msg = self.serial.readline().strip().decode()
        self.serial.timeout = 10 * ms

        # Ensure that firmware and driver have the same version
        msg_template = "Arduino is ready. Firmware version: "
        if msg != msg_template + __version__:
            if msg.find(msg_template) != 0:
                raise ConnectionError(
                    f"Could not connect to Arduino on port {port};\n"
                    + f"Received message:\n{msg}\n"
                    + f"Expected message:\n{msg_template + __version__}"
                )
            v = msg[len(msg_template) :]

            version_match = _compare_versions(v, __version__)
            if not version_match["major"] or not version_match["minor"]:
                raise RuntimeWarning(
                    f"Version mismatch: driver {__version__} != firmware {v}"
                )

        self.serial.reset_input_buffer()
        sleep(100 * ms)

        self._transaction_mode_ = False
        self._buffer_ = bytearray()

    def __del__(self):
        self.serial.close()

    def __enter__(self):
        """
        Enter a transaction - cache all outgoing data in a buffer
        """
        self._transaction_mode_ = True
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """
        Exit a transaction - transmit all cached data to the microcontroller
        """
        self.serial.write(self._buffer_)
        self._buffer_ = bytearray()
        self._transaction_mode_ = False

    def get_register(self, register: RegisterBase):
        if register.bits == 8:
            return self._get_8bit_register(register.addr)
        elif register.bits == 16:
            return self._get_16bit_register(register.addr)
        else:
            ValueError(f"Unknown register {register}")

    def set_register(self, register: RegisterBase, x):
        if register.bits == 8:
            self._set_8bit_register(register.addr, x)
        if register.bits == 16:
            self._set_16bit_register(register.addr, x)
        else:
            ValueError(f"Unknown register {register}")

    def _get_8bit_register(self, addr):
        self.serial.reset_input_buffer()
        self.serial.write(_pad(bytearray([ord("R"), addr])))
        return ord(self.serial.read(1))

    def _get_16bit_register(self, addrL):
        byte_L = self._get_8bit_register(addrL)
        byte_H = self._get_8bit_register(addrL + 1)
        return byte_H << 8 | byte_L

    def _set_8bit_register(self, addr, x):
        cmd = bytearray(_pad([ord("W"), addr, x]))
        if self._transaction_mode_:
            self._buffer_ += cmd
            if len(self._buffer_) > 63:
                raise MemoryError("Buffer overflow: more than 64 bytes of data cached.")
        else:
            self.serial.write(cmd)

    def _set_16bit_register(self, addrL, x):
        byte_L = x & 0xFF
        byte_H = x >> 8
        self._set_8bit_register(addrL + 1, byte_H)
        self._set_8bit_register(addrL, byte_L)

    def start_trigger(self, exp_time_ms, n_frames=0, cam_delay_ms=0, inj_delay_ms=0):
        """
        Configure and start the camera trigger (timer/counter 1).
        """
        uint16 = lambda x: bytes(ctypes.c_ushort(x))
        uint16_cts = lambda time_ms: uint16(int(time_ms * ms * 16 * MHz / 1024))
        self.serial.write(
            _pad(
                b"S"
                + uint16_cts(exp_time_ms)
                + uint16(n_frames)
                + uint16_cts(cam_delay_ms)
                + uint16_cts(inj_delay_ms)
            )
        )

    def stop_trigger(self):
        """
        Stop running camera trigger
        """
        self.serial.write(_pad(b"Q"))

    def set_exposure(self, exp_time_ms):
        """
        Set exposure time of the camera trigger. Does not start or stop the trigger,
        but would immediate change interrup a running trigger and change its period
        on the fly.
        """
        uint16 = lambda x: bytes(ctypes.c_ushort(x))
        uint16_cts = lambda time_ms: uint16(int(time_ms * ms * 16 * MHz / 1024))
        self.serial.write(_pad(b"E" + uint16_cts(exp_time_ms)))

    def _bitlist2int(self, bitlist, rev=False):
        """Convert list of bits into integer. Example: [1, 1, 0, 1, 0] => b11011"""
        out = 0
        if rev:
            bitlist = reversed(bitlist)
        for bit in bitlist:
            out = (out << 1) | bit
        return out

    def set_shutters(self, active, idle=None, ALEX=False):
        """
        Set shutter state for active and idle cases.
        If idle is not provided, it will be the inverse of active.
        Expected input: list of four values, order: cy2, cy3, cy5, cy7.
        """
        a = self._bitlist2int(active, rev=True)
        i = self._bitlist2int(idle, rev=True) if idle is not None else 0xFF - a
        ALEX = 1 if ALEX else 0
        self.serial.write(_pad(bytearray([ord("L"), a, i, ALEX])))


def define_AVR(RegisterList: RegisterBase):
    """
    Creates class representing a specific microcontroller based on a list of registers
    """

    class AVR(AVR_Base):
        def __init__(self, port, baudrate=2 * MHz):
            super().__init__(port, baudrate)

    for register in RegisterList:

        def _getter(self, register=register):
            return self.get_register(register)

        def _setter(self, x, register=register):
            self.set_register(register, x)

        prop = property(fget=_getter, fset=_setter)

        setattr(AVR, register.name, prop)

    return AVR
