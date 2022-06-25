# avr-python-control
Low-level control of an AVR microcontroller using Python via serial port.

Each AVR microcontroller has a number of registers that control the specific behavior of the device subsystems. Since these registers are mapped to the memory, we actually don't have to hardcode their addresses in C++. Instead, we can send the address of a given register over the serial port! The register addresses and the meaning of their bits are listed in the section "Register summary" of each AVR datasheet.

This project consists of a very simple firmware, a primitive communication protocol, and a Python interface.

## Communication protocol

Each command is transmitted as a `\n`-terminated data packet with up to 5 bytes in length:

```
Byte # | value     | meaning                                     |
-------|-----------|---------------------------------------------|
1      | R or W    | Read or write                               |
2 & 3  | ascii hex | Register address                            |
4 & 5  | ascii hex | Register value to write (ignored for read)  |
```

Example: `WA71F\n` means "write value `0x1F` to register with the address of `0xA7`. `R24\n` is a command to read the content of register with the address `0x24`.

## Firmware

The simplified pseudocode of the firmware is:

```python
serial.start(baudrate = 2*Mbps)

if serial.has_data:
    buf = serial.read_until('\n', max_bytes = 5)
    cmd = buf[0]
    addr = int(buf[1:2], 16)
    if cmd == 'W':
        register_at(addr) = int(buf[3:4], 16)
    elif cmd == 'R":
        serial.write("r" + addr + hex(register_at(addr)))
```

## Python driver

The driver makes the communication human-readable. Instead of sending plain numbers, we now operate with register and bit names.

```python
avr = AVR(serial="COM3")
avr.TCCR1A = WGM1 | WGM2 | COM1A1  # initialize timer 1
print("{:08b}".format(avr.SREG)    # print the status register
```

If you know how to use registers and where to find information about them, you can quickly build more complex scenarios and interfaces on top of this. The best part - the functionality can be defined and changed dynamically from Python, without touching the actual firmware of the microcontroller.

