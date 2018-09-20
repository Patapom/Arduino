## Pom Library

This is a Visual Studio project containing a new library with advanced helpers.

It contains:

* A refactor for various libraries:
** TWI, a nice class to manage Two-Wires Interface/I2C. A bit clearer than the existing "Wire" implementation.

-----

* General helpers like:
** Byte swapping
** Formatted serial printf
** PulseTrain helper that records a series of LOW-state pulses until a HIGH-state pulse of a given length is encountered, in which case the packet is deemed finished and the routine returns
 It behaves pretty much like the well-known pulseIn() except it records multiple pulses before returning.
** Buffered Read, a helper that reads the inputs of a digital pin and only returns true if it read the expected value (HIGH or LOW) for a significant amount of time.
 It helps ignoring single noise spikes on sensitive inputs (useful for reading the rotary button switch for example).

-----

* Advanced math helpers like:
** log2
** Values clamping

-----

* Some Device Drivers for:
** CC1101, a RF chip
** MCP4725, a simple DAC