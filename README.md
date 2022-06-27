# avr-python-control
Low-level control of an AVR microcontroller using Python via serial port.

Each AVR microcontroller has a number of registers that control the specific behavior of the device subsystems. Since these registers are mapped to the memory, we actually don't have to hardcode their addresses in C++. Instead, we can send the address of a given register over the serial port! The register addresses and the meaning of their bits are listed in the section "Register summary" of each AVR datasheet.

This project consists of a very simple firmware, a primitive communication protocol, and a Python interface.

## Communication protocol

Each command is transmitted from PC to the microcontroller as a 3-byte packet:

```
Byte | Value   | Description                                 |
-----|---------|---------------------------------------------|
1    | cmd     | Command: R-read / W-write. Can be extended  |
2    | addr    | Register address                            |
3    | value   | Register value to write (ignored for read)  |
```

Examples:
  * `W\xA7\x1F` means "write value `0x1F` to register with the address of `0xA7`.
  * `R2` requests value of register with the address `0x32` (hex code of symbol `2`).
  * `R24` same as above; the last byte, `4`, is ignored for the read operation.

The result of the `R` (read) command is just one byte - the content of the register.

The protocol could be extended with additional commands that would invoke user-defined firmware functions.

Once any data arrives to the microcontroller serial port, it will try to read
all three bytes at once, waiting up to 10 ms for each byte. If a byte gets lost,
the whole frame gets discarded - this ensures that the devices don't run out of sync.

## Firmware

The simplified pseudocode of the firmware is:

```python
serial.start(baudrate = 2*Mbps)

if serial.has_data:
    cmd, addr, value = serial.read(n_bytes = 3)
    if cmd == 'W':
        register_at(addr) = value
    elif cmd == 'R":
        serial.write(register_at(addr))
```

## Python driver

The driver makes the communication human-readable. Instead of sending plain numbers, we now operate with register and bit names.

```python
avr = AVR(serial="COM3")
avr.TCCR1A = WGM1 | WGM2 | COM1A1  # initialize timer 1
print("{:08b}".format(avr.SREG))   # print the status register
```

If you know how to use registers and where to find information about them, you can quickly build more complex scenarios and interfaces on top of this. The best part - the functionality can be defined and changed dynamically from Python, without touching the actual firmware of the microcontroller.

## Initialization

Remember that on Arduino boards the AVR gets reset when you open the serial port. Therefore, you might have to wait for it to restart.
