
------
 This documentation is a summary of how I understand the Arduino software architecture is working as I go along in my investigations.
  
	 Date Format Is	YYYY-MM-DD
	 Creation Date:	2017-08-04
	 Last Update:	2018-08-13
	 Author:		Patapom (www.patapom.com)
					@patapom2 (twitter)

------

# Acronyms and Names

* *Arduino*, a combination of both chips and board.
* *Atmel*, the company that builds the micro-controller (http://www.atmel.com/)
* *AVR*, the central micro-controller on the Arduino boards, made by Atmel. List of chips: http://www.atmel.com/products/microcontrollers/avr/megaAVR.aspx
* *CDC*, Communication Device Class (https://en.wikipedia.org/wiki/USB_communications_device_class)
* *ISP*, In-System Programmer. Apparently it's the fact of using an Arduino to program another micro-controller.
* *I2C* (or I²C), "Inter-Integrated Circuit" is typically used for attaching lower-speed peripheral ICs to processors and microcontrollers in short-distance, intra-board communication (https://en.wikipedia.org/wiki/I%C2%B2C)
* *MISO*, Master In Slave Out
* *MOSI*, Master Out Slave In
* *PCD*, Proximity Coupling Device e.g. RFID contactless reader/writer like MFRC522 (check public datasheet)
* *PICC*, Proximity Integrated Circuit Card: a card or tag using the ISO 14443A interface, e.g. Mifare or NTAG203.
* *RSSI*, Received Signal Strength Indication is a measurement of the power present in a received radio signal.
* *SCK*, Serial Clock
* *SPI*, Serial Peripheral Interface. is a synchronous serial data protocol used by microcontrollers for
		communicating with one or more peripheral devices quickly over short distances.
		It can also be used for communication between two microcontrollers. (https://www.arduino.cc/en/Reference/SPI)
* *SS*, Slave Select
* *TWI*, Two Wires Interface. Apparently, exactly the same thing as I2C except it has a different name for copyright reasons.
* *UART*, Universal Asynchronous Receiver Transmitter (e.g. Motorola 6850)


# Chips on the Arduino Boards

* Arduino UNO:
	* *ATMega328P-PU*: the central AVR micro-controller. 8-bit picoPower AVR Microcontroller, 32KB Flash EEPROM, 2KB RAM. 28/32-pin. Digital Communication Peripherals > 1-UART, 2-SPI, 1-I2C
	* *ATmega8U2*: the USB controller. 8-bit AVR Microcontroller, 8KB Flash EEPROM, 32-pin, USB Controller. Digital Communication Peripherals > 1-UART, 2-SPI


# Folder Hierarchy

 _Root Directory_
	It is the Install Directory, on my machine it's: [arduinodir] = "c:\program files (x86)\Arduino"
	You can find the following subdirectories:

* [arduinodir]\drivers

	This folder apparently contains drivers for USB ports communication with the Arduino boards

* [arduinodir]\examples

	Contains the various tutorial projects (.ino files + .txt doc + image of board configuration)

* [arduinodir]\java

	Certainly used by the Arduino.exe front end, not important for root usage

* [arduinodir]\lib

	Java and resource files for the Arduino.exe front end, not important for root usage

* [arduinodir]\libraries

	"High-Level" libraries for various devices and sensors (IR, temperature, servo, etc.)
	This is interesting to show how to write additional libraries but these are not "core libraries" since
		they're only a bunch of .CPP/.H files that get included and compiled along your main .INO file when
		included by your project.

* [arduinodir]\reference

	A bunch of help files.

* [arduinodir]\tools

	A tool seems to be a java plug-in for the Arduino.exe front end and I believe it's not important for root usage.
		A summary of what a tool is can be found in the "howto.txt" file at the root of this folder:
		« A Tool is a chunk of code that runs from the Tools menu. Tools are a means 
			of building onto the Processing Development Environment without needing to
			rebuild the beast from source. »

* [arduinodir]\tools-builder

	No idea what that is. Maybe the library that is capable of building a user tool?


* [arduinodir]\hardware
	This is the directory that will interest us the most for root usage of the arduino boards!
		
	* arduino\tools\
		
		TODO
		
	* arduino\avr\
		* arduino\avr\bootloaders
			Contains some bootloader in .C and .HEX files for the various boards

		* arduino\avr\cores\  (arduino\)
			**This is the core library files!**

			Most notably:

				* Contains the main "arduino.h" file defining the core functions like pinMode, analogRead, pulseIn, etc.
				* Contains the "main.cpp" with the void main() {} loop that calls the user setup() and indefinitely calls the user loop() functions
				
		* arduino\avr\firmwares
			Contains firmwares for the various onboard chips like:

				* arduinoISP, In-System Programmer... Not clear. Must be for programming other boards from an arduino board.
				* atmegaxxu2, firmware for the ATmega8U2 USB controller.
				* wifishield, firmware for the wifi-shield that can be plugged into the board for wifi communication.

		* arduino\avr\libraries
			Apparently, contains low-level libraries to interact with the various board components...
			For example, the Readme.md found in EEPROM writes:

				« The EEPROM library provides an easy to use interface to interact with the internal non-volatile storage found
				in AVR based Arduino boards. This library will work on many AVR devices like ATtiny and ATmega chips. »

		* arduino\avr\variants\<board variant>\pins_arduino.h
			The only file that varies with the board. It defines the various pins of the board.

		* arduino\avr\boards.txt
			Contains many definitions for all the existing Arduino Boards
			For example, the Uno MCU is the "Atmega328p"
		
		
# VMicro

VMicro is the Visual Studio plug-in that allows us to build and run (and "debug") an arduino executable to send it to the board for writing.

## Subtleties

* VMicro auto-generates a file named "__vm/.<Project Name>.vsarduino.h" every compilation. It includes the "<Project Name>.INO" file at the very end
	
* The "<Project Name>.INO" files are nothing more than C++ header files.
	* They **MUST** exist for every project!
	* They **MUST** be kept at the root of the project: even though C++ Visual Studio project filters
		are virtual folders without any existence on the disk, VMicro doesn't seem to find the INO file otherwise.
	* They **MUST** contain the setup() function. Otherwise nothing works: no **serial**, no nothing...
		 I guess VMicro attempts to poorly patch the exe assuming the functions have to be there... :/
	* Serial.begin() **MUST** be called in the setup() function in the INO file or it won't fire

* It's easy to revert to a usual C++ project by making the "<Project Name>.INO" a single line:
	#include "MyRootHeader.h"

* Output files are found in:
	C:\Users\<User Name>\AppData\Local\Temp\VMBuilds\<Project Name>\uno\<ConfigName>\
	Example: C:\Users\Patapom\AppData\Local\Temp\VMBuilds\BlinkFaster\uno\Debug

* We notice that ALL files are compiled, including the "core library" files since we find files like "main.cpp.o" and "Stream.cpp.o" that are the compiled versions of the files found in the arduino\avr\cores directory!

* I'm still wondering how it knows where to find the "core files"?
	* What if I want to add my own root library? Do I have to adde my files to the "<INSTALL DIR>\arduino\avr\" directory?
	* I think I found the solution!!!
		1) Install http://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html
		2) Pick the folder of your custom library as "link source"
		3) Drop as junction into the "<INSTALL DIR>\arduino\avr\cores\" directory
		4) Voilà! VMicro will compile your cpp files along regular core library files each time!

