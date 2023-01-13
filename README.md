# Synchronization device for laser and camera control at pTIRF microscope

The firmware is developed using Microchip Studio.

I'm going to use Mega2560 microcontroller (Arduino Mega v3 board) that has four 16bit timers for generation of precise TTL signals.


## How it works

### System states

The system can be in the following states:

 * IDLE - no signals. Timers 1 and 3 are not running
 * Stroboscopic imaging (potentially with ALEX). Tell the system what lasers to use and how many frames to acquire. QUESTION: do we need stroboscopic with 1+ lasers at the same time (no alternation)??
 * Continuous imaging (NOT IMPLEMENTED). Laser(s) stay on all the time, we just trigger the camera. First frame is at most 200ms long (throw away frame).


### Communication protocol (Rudimentary)

 Each data packet is 5 byte long, which should be transmitted together without big delays between them. The first byte is the command, other 4 bytes are arguments (usually in uint32 format). Most of the arguments map directly to the `SystemSettings` structure of the microcontroller firmware to modify the relevant fields. 




## Installation

### Firmware

You'll need Microchip Studio (free) to build the firmware. Open the solution file and build the release target. To upload the firmaware to the microcontroller,
you can use the embedded Arduino programmer. It is not supported out of the box by the Microchip studio but it's [pretty straightforward to add it](https://youtu.be/zEbSQaQJvHI).

Brifly, open Arduino IDE, in preferences set "show verbose output when uploading", and locate the COM port number corresponding to your microcontroller (menu Tools->Port). Run a test upload and pay attention to how `avrdude` was invoked in the command line.
Then got to Microchip Studio and open menu Tools->External Tools. Register ArduinoBootloader tool where the executable is

```
C:\Program Files (x86)\Arduino\hardware\tools\avr/bin/avrdude.exe
```

and the arguments are (update them if necessary, especially pay attention to the microcontroller type and COM port)

```
-C"C:\Program Files (x86)\Arduino\hardware\tools\avr/etc/avrdude.conf" -v -patmega2560 -cwiring -PCOM11 -b115200 -D -Uflash:w:"$(ProjectDir)Release\$(TargetName).hex":i 
```

Make sure that the build target is "Release" and not "Debug".


### Python module

Install with pip in a virtual environment.

```
pip install -e .
```
