# Synchronization device for laser and camera control at pTIRF microscope

The firmware is developed using Microchip Studio.

I'm going to use Mega2560 microcontroller (Arduino Mega v3 board) that has four 16bit timers for generation of precise TTL signals.


## Installation

You'll need Microchip Studio (free) to build the firmware. Open the solution file and build the release target. To upload the firmaware to the microcontroller,
you can use the embedded Arduino programmer. It is not supported out of the box by the Microchip studio but it's [pretty straightforward to add it](https://youtu.be/zEbSQaQJvHI).

Brifly, open Arduino IDE and locate the COM port number corresponding to your microcontroller (menu Tools->Port). Then got to Microchip Studio and open menu Tools->External Tools. Register ArduinoBootloader tool where the executable is

```
C:\Program Files (x86)\Arduino\hardware\tools\avr/bin/avrdude.exe
```

and the arguments are (update microcontroller type and COM port)

```
-C"C:\Program Files (x86)\Arduino\hardware\tools\avr/etc/avrdude.conf" -v -patmega2560 -carduino -PCOM8 -b115200 -D -Uflash:w:"$(ProjectDir)Debug\$(TargetName).hex":i 
```
