from argparse import ArgumentError
from cProfile import run
from numpy import byte
from serial import Serial
from enum import Enum
from time import sleep
from .__version__ import __version__
from .constants import ms, MHz
from ctypes import c_uint16
from ctypes import c_uint8


def c16(value):
    assert value < 2**16, "value exceeds 16 bit"
    return c_uint16(value)


def c8(value):
    assert value < 2**8, "value exceeds 8 bit"
    return c_uint8(value)


class SyncDeviceError(ValueError):
    def __init__(self, reply, message="Incorrect args supplied to the sync device."):
        self.reply = reply
        self.message = message + "\nDevice reply:\n -> " + reply
        super().__init__(self.message)


class Port(Serial):
    def __enter__(self):
        self.reset_input_buffer()
        return self

    def __exit__(self, *args, **kwargs):
        reply = self.readline().strip().decode()
        if reply == "OK":
            return True
        raise SyncDeviceError(reply)


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


def pad(data: bytearray, length=5):
    return data + bytearray([0] * (length - len(data)))


def cts16(value):
    cts = int(value * ms * 16 * MHz / 1024)
    assert cts < 2**16, "Number of counts exceeds 16bit"
    return c16(cts)


class AVR_Base(object):
    """
    Abstract AVR microcontroller unit (MCU). You can read/write registers if you know their address.
    However, it is recommended to derive a class for a specific MCU by calling `define_AVR` with list of registers
    """

    def __init__(self, port, baudrate):
        self.com = Port(port, baudrate=baudrate)
        if not self.com.is_open:
            self.com.open()

        # Opening of the serial port resets Arduino
        # We are going to wait for it to start up
        self.com.timeout = 5  # seconds
        msg = self.com.readline().strip().decode()
        self.com.timeout = 10 * ms

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

        self.com.reset_input_buffer()
        sleep(100 * ms)

        self._transaction_mode_ = False
        self._buffer_ = bytearray()

    def __del__(self):
        self.com.close()

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
        self.com.write(self._buffer_)
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
        self.com.reset_input_buffer()
        self.com.write(pad(b"R" + c8(addr)))
        return ord(self.com.read(1))

    def _get_16bit_register(self, addrL):
        byte_L = self._get_8bit_register(addrL)
        byte_H = self._get_8bit_register(addrL + 1)
        return byte_H << 8 | byte_L

    def _set_8bit_register(self, addr, x):
        cmd = pad(b"W" + c8(addr) + c8(x))
        if self._transaction_mode_:
            self._buffer_ += cmd
            if len(self._buffer_) > 63:
                raise MemoryError("Buffer overflow: more than 64 bytes of data cached.")
        else:
            self.com.write(cmd)

    def _set_16bit_register(self, addrL, x):
        byte_L = x & 0xFF
        byte_H = x >> 8
        self._set_8bit_register(addrL + 1, byte_H)
        self._set_8bit_register(addrL, byte_L)

    def set_fluidics_delay(self, delay_ms=0):
        """
        Set delay between the fluidic trigger and the start of the timer
        """
        with self.com as com:
            com.write(pad(b"F" + c16(delay_ms)))

    def start_stroboscopic_acquisition(
        self, exp_time_ms, n_frames=0, interframe_time=0, timelapse_delay=0
    ):
        """
        Configure and start the camera trigger (timer/counter 1) in the stroboscopic / ALEX / timelapse mode
        """
        with self.com as com:
            com.write(
                pad(
                    b"S"
                    + cts16(exp_time_ms)
                    + c16(n_frames)
                    + cts16(interframe_time)
                    + c16(timelapse_delay)
                )
            )

    def start_continuous_acquisition(self, exp_time_ms, n_frames=0):
        """
        Configure and start the camera trigger (timer/counter 1).
        """
        with self.com as com:
            com.write(pad(b"C" + cts16(exp_time_ms) + c16(n_frames)))

    def stop(self):
        """
        Stop running camera trigger
        """
        with self.com as com:
            com.write(pad(b"Q"))

    def set_exposure(self, exp_time_ms):
        """
        Set exposure time of the camera trigger. Does not start or stop the trigger,
        but would immediate change interrup a running trigger and change its period
        on the fly.
        """
        with self.com as com:
            com.write(pad(b"E" + cts16(exp_time_ms)))

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
        with self.com as com:
            com.write(pad(b"L" + c8(a) + c8(i) + c8(ALEX)))


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
