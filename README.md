# Synchronization Device for Laser and Camera Control at pTIRF Microscope

This project uses Arduino Mega2560 rev3 microcontroller board for generation of precise TTL signals to trigger camera and laser shutters in pTIRF microscopy instrument.
ATMega2560 has four 16bit hardware timers that run independently from the computing core and provide configurable hardware interrupts that are called at predefined time points.
This mechanism allows generation of precise jitter-free waveforms with submillisecond resolution.

The project is based on Arduino hardware but does _not_ use the Arduino library due to overhead it introduces. The minimalistic firmware is developed in low-level C++ using Microchip Studio.

## High-level Overview

We use hardware 16-bit timer/counters 1 and 3 (`TCNT1` and `TCNT3`) with four corresponding interrupt vectors (compare matches A, B, C, and overflow event) for generation of signals, and hardware UART module for communication with the host computer. The UART module logic runs in the background and does not rely on interrupts to prevent any jitter in the generated signals.

`sync_device` is a state machine with a set of global variables (timings, spectral channels, etc) that control its behavior. The variables are set by the software on the host computer via UART communication protocol (see corresponding section below). The values of global variables persist from acquisition to acquisition. Generally, we distinguish between three global system states:

 * `IDLE` - no signals. Hardware timer/counters 1 and 3 are not running.
 * `STRB_ACQ` - stroboscopic/ALEX/timelapse imaging (see waveforms below). The CMOS camera runs in the _level trigger_ mode. During each each camera exposure the sample is briefly illuminated with the laser, following by camera readout, and (optional) waiting period for timelapse. If ALEX is enabled, imaging is done in bursts of frames, where each frame within the burst is illuminated by a different laser. The might be an optional waiting period between bursts for timelapse imaging.
 * `CONT_ACQ` - continuous imaging. The CMOS camera is in the _synchronous readout_ mode. The laser shutter(s) is/are open during the entire acquisition, and only camera is triggered. The first camera frame must be thrown away as it contains signal accumulated before the acquisition has started.


## Waveforms
### Stroboscopic Imaging

![Stroboscopic Imaging](doc/waveforms/stroboscopic%20imaging.svg)

Before starting the stroboscopic imaging, it is essential to correctly set the following system variables (in microseconds):

* `shutter_delay_us` - time it takes for the laser shutter to fully open (default: 1ms).
* `exp_time_us` - duration of the laser pulse, i.e. exposure time (default: 5ms).
* `cam_readout_us` - duration of camera readout (default: 12ms).
* `acq_period_us` - time period between two subsequent frames or bursts of frames, for timelapse imaging (default: 100ms).
* `lasers` and `ALEX` - binary mask indicating what lasers are in use, and whether ALEX mode is active or not (default: all four lasers, ALEX is on).

Once the `S` command and requested number of frames is sent, both `TCNT1` and `TCNT3` start counting from zero. `TCNT1` handles events happening within one frame:

* Overflow interrupt `TIMER1_OVF` routine opens laser shutter (raising edge). It also counts number of acquired frames.
* Interrupt `COMPA` starts camera exposure (raising edge).
* Interrupt `COMPB` closes laser shutter (falling edge).
* Interrupt `COMPC` stops camera exposure and triggers readout (falling edge). It also pauses `TCNT1` unless it happens in within the frame burst in ALEX mode. In this case, it handles logic of cycling through laser channels.

The period of `TCNT` is equal to `shutter_delay_us` + `exp_time_us` + `cam_readout_us`, and has to be smaller than 4.19s (current limitation). The `acq_period_us` can be any value.

In parallel with `TCNT1`, `TCNT3` keeps track of the `acq_period_us` - time period between two subsequent frames or bursts of frames. The overflow interrupt `TIMER3_OVF` routine checks whether  the requested number of frames has been acquired. If so, it stops acquisition, sends "DONE\n" to the host, and stops both `TCNT1` and `TCNT3`. Otherwise, it starts `TCNT1`, triggering new frame or burst of frames. The overflow interrupt `TIMER3_OVF` routine has logic to correctly work with long `acq_period_us` time intervals (above 4.2s).




### Communication protocol (Rudimentary)

 Each data packet is 5 byte long, which should be transmitted together without big delays between them. The first byte is the command, other 4 bytes are arguments (usually in uint32 format). Most of the arguments map directly to the `SystemSettings` structure of the microcontroller firmware to modify the relevant fields. 


![data packet structure.png](doc/data%20packet%20structure.png)



## Installation


## Wiring

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


# Known issues

* No sanity checks are done on the provided timing values (yet). The system would not send back `ERR` response for nonsense inputs and will silently generate an incorrect waveform.
* Maximum exposure time is currently limited to 4.19 seconds. The value is defined by system oscillator running at 16 MHz, 1/1024 prescaler providing clock pulses every 64µs, and the size of 16-bit timer that can count up to 65536× 64µs pulses. There is no upper limit on the acquisition period for timelapse, though.
* The timing intervals are changed in 64µs steps, which is very coarse for sub-millisecond imaging. The hardware prescaler supports finer quantization intervals but the implementation is quite challenging.