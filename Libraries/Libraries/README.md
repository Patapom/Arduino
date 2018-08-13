## AVR Library

This is a Visual Studio project containing all the files located in <ARDUINO INSTALL DIRECTORY>\hardware\arduino\avr\libraries

This small library contains medium-level helpers that build upon the low-level AVR library, essentially some interfacing- and communication-related stuff like:

* Serial Peripheral Interface (SPI), full-duplex communication with embedded systems using the well-known (SCLK, MOSI, MISO, SS) four-wires protocol (https://en.wikipedia.org/wiki/Serial_Peripheral_Interface)
* Two-Wires Communication (TWI) / Inter-Integrated Circuit (I²C), low-speed easy to integrate two-wires protocol (https://en.wikipedia.org/wiki/I%C2%B2C)
* Human Interface Devices (HID) like mouse and keyboard communication
* Serial communication helpers
* EEPROM programmation helpers
