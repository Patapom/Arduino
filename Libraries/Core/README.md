## Core Library

This is a Visual Studio project containing all the files located in <ARDUINO INSTALL DIRECTORY>\hardware\arduino\avr\cores\arduino

Contains all the important tiny helper functions and macros declared by the include to Arduino.h like:

* pinMode
* digitalRead/digitalWrite
* analogRead/analogWrite
* delay/millis/micros/pulseIn
* shiftOut/shiftIn
* attachInterrupt/detachInterrupt
* tone


Also contains the **entry point** main() function that will in turn call this:

	init();
	initVariant();

	#if defined(USBCON)
		USBDevice.attach();
	#endif
	
	setup();
    
	for (;;) {
		loop();


This is also a higher-level library built upon the low-level AVR LibC library that offers near-C++-level helpers like:

* The C++ new/delete operators
* Advanced string class manipulation
* Abstract Stream class
** Including the HardwareSerial stream through which to communicate when the Arduino board is plugged to the PC via USB
* USB Device API, to control devices by USB, including the USB micro-controller on the Arduino board itself
** CDC "Communication Device Class" is a composite Universal Serial Bus device class.
* IP Address class, to easily manipulate IP addresses
