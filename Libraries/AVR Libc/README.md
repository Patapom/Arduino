## AVR LibC v2.0.0

Contains the Visual Studio project gathering the sources of the AVR LibC library https://www.nongnu.org/avr-libc/
Online documentation is available here: https://www.nongnu.org/avr-libc/user-manual/index.html

The AVR LibC Library contains pretty much everything the standard C library can offer on a regular system and is used as the basic scaffolding for any other code:

* Support for integer and floating-point types
* Basic String & Character manipulation (string.h)
* Stdlib.h, Stdio.h
* Memory allocation
* Complex math functions (math.h)
* Various AVR helpers (CRC16, delay functions, TWI/I²C)

It's very basic and more advanced libraries (like the one provided when installing the Arduino SDK, or my "Pom Library"), build upon this.
