
 This documentation is a summary of how I understand the Arduino software architecture is working as I go along in my investigations.
  
    Date Format Is:	YYYY-MM-DD
    Creation Date:	2017-08-04
    Last Update:	2018-08-13
    Author:			Patapom (www.patapom.com)
    				@patapom2 (twitter)


# Acronyms and Names

* **Arduino**, a combination of both chips and board.
* **Atmel**, the company that builds the micro-controller (http://www.atmel.com/)
* **AVR**, the central micro-controller on the Arduino boards, made by Atmel. List of chips: http://www.atmel.com/products/microcontrollers/avr/megaAVR.aspx
* **CDC**, Communication Device Class (https://en.wikipedia.org/wiki/USB_communications_device_class)
* **SFR**, Special Function Registers. Allow access to special I/O registers. Check the "Memory Map" chapter below.
* **ISP**, In-System Programmer. Apparently it's the fact of using an Arduino to program another micro-controller.
* **I2C** (or I²C), "Inter-Integrated Circuit" is typically used for attaching lower-speed peripheral ICs to processors and microcontrollers in short-distance, intra-board communication (https://en.wikipedia.org/wiki/I%C2%B2C)
* **MISO**, Master In Slave Out
* **MOSI**, Master Out Slave In
* **PCD**, Proximity Coupling Device e.g. RFID contactless reader/writer like MFRC522 (check public datasheet)
* **PICC**, Proximity Integrated Circuit Card: a card or tag using the ISO 14443A interface, e.g. Mifare or NTAG203.
* **RSSI**, Received Signal Strength Indication is a measurement of the power present in a received radio signal.
* **SCK**, Serial Clock
* **SPI**, Serial Peripheral Interface. is a synchronous serial data protocol used by microcontrollers for
			communicating with one or more peripheral devices quickly over short distances.
			It can also be used for communication between two microcontrollers. (https://www.arduino.cc/en/Reference/SPI)
* **SS**, Slave Select
* **TWI**, Two Wires Interface. Apparently, exactly the same thing as I2C except it has a different name for copyright reasons.
* **UART**, Universal Asynchronous Receiver Transmitter (e.g. Motorola 6850)


# Chips on the Arduino Boards

* Arduino UNO:
	* *ATMega328P-PU*: the central AVR micro-controller. 8-bit picoPower AVR Microcontroller, 32KB Flash EEPROM, 2KB RAM. 28/32-pin. Digital Communication Peripherals > 1-UART, 2-SPI, 1-I2C
	* *ATmega8U2*: the USB controller. 8-bit AVR Microcontroller, 8KB Flash EEPROM, 32-pin, USB Controller. Digital Communication Peripherals > 1-UART, 2-SPI


# Memory Map

From section "8.3 SRAM Data Memory" of the Arduino UNO ["ATMega328P" datasheet](./Doc/Atmel/Atmel-8271-8-bit-AVR-Microcontroller-ATmega48A-48PA-88A-88PA-168A-168PA-328-328P_datasheet_Complete.pdf), the *ATMega328P* contains 2 memory spaces:

* The program memory space

	32KB Reprogrammable Flash memory organized as **16KWords** where a word is 16-bits.
	The Program Counter (PC) of the ATMega328P is only 14 bits wide so it can only address up to 16KW of program memory indeed.

	The memory is seprated into 2 sections:
		1) The boot loader section
		2) The application program section

	At the beginning of the Application Program section you will find the 16-bits interrupt vectors,
	 which are basically indirect addresses to jump to if an interrupt signal is received. 
	The *order* of the interrupts indicates their priority, the RESET interrupt always being the highest.

	Vector No.	|  Program Address	|  Source		| Interrupt Definition
	-------------------------------------------------------------------
		01		|		0x0000 (1)	| RESET			| External Pin, Power-on Reset, Brown-out Reset and Watchdog System Reset
		02		|		0x0002		| INT0			| External Interrupt Request 0
		03		|		0x0004		| INT1			| External Interrupt Request 1
		04		|		0x0006		| PCINT0		| Pin Change Interrupt Request 0
		05		|		0x0008		| PCINT1		| Pin Change Interrupt Request 1
		06		|		0x000A		| PCINT2		| Pin Change Interrupt Request 2
		07		|		0x000C		| WDT			| Watchdog Time-out Interrupt
		08		|		0x000E		| TIMER2		| COMPA Timer/Counter2 Compare Match A
		09		|		0x0010		| TIMER2		| COMPB Timer/Counter2 Compare Match B
		10		|		0x0012		| TIMER2		| OVF Timer/Counter2 Overflow
		11		|		0x0014		| TIMER1		| CAPT Timer/Counter1 Capture Event
		12		|		0x0016		| TIMER1		| COMPA Timer/Counter1 Compare Match A
		13		|		0x0018		| TIMER1		| COMPB Timer/Coutner1 Compare Match B
		14		|		0x001A		| TIMER1		| OVF Timer/Counter1 Overflow
		15		|		0x001C		| TIMER0		| COMPA Timer/Counter0 Compare Match A
		16		|		0x001E		| TIMER0		| COMPB Timer/Counter0 Compare Match B
		17		|		0x0020		| TIMER0		| OVF Timer/Counter0 Overflow
		18		|		0x0022		| SPI, STC		| SPI Serial Transfer Complete
		19		|		0x0024		| USART, RX		| USART Rx Complete
		20		|		0x0026		| USART, UDRE	| USART, Data Register Empty
		21		|		0x0028		| USART, TX		| USART, Tx Complete
		22		|		0x002A		| ADC			| ADC Conversion Complete
		23		|		0x002C		| EE READY		| EEPROM Ready
		24		|		0x002E		| ANALOG COMP	| Analog Comparator
		25		|		0x0030		| TWI			| 2-wire Serial Interface
		26		|		0x0032		| SPM READY		| Store Program Memory Ready

	(1) When the BOOTRST Fuse is programmed, the device will jump to the Boot Loader address at reset


* The SRAM data memory space split between:
	* Special Function Registers (SFR) in the range [0x00, 0xFF]
		* Range [0x00,0x1F] = Register File

			0x00 => R00\
			0x01 => R01\
			0x02 => R02\
			   (...)   \
			0x18 => R24\
			0x19 => R25\
			0x1A => XL		<= 3 Address registers for indirect addressing of data space\
			0x1B => XH\
			0x1C => YL\
			0x1D => YH\
			0x1E => ZL\
			0x1F => ZH\


		* Range [0x20,0x5F] = I/O Memory
			0x23 => PINB	(The Port B Input Pins Address, section 14.4.4)\
			0x24 => DDRB	(The Port B Data Direction Register, section 14.4.3)\
			0x25 => PORTB	(The Port B Data Register, section 14.4.2)\
			0x26 => PINC	(The Port C Input Pins Address, section 14.4.7)\
			0x27 => DDRC	(The Port C Data Direction Register, section 14.4.6)\
			0x28 => PORTC	(The Port C Data Register, section 14.4.5)\
			0x29 => PIND	(The Port D Input Pins Address, section 14.4.10)\
			0x2B => DDRD	(The Port D Data Direction Register, section 14.4.9)\
			0x2C => PORTD	(The Port D Data Register, section 14.4.8)\
			   (...)\
			0x35 => TIFR0	(Timer/Counter 0 Interrupt Flag Register, section 15.9.7)\
			0x36 => TIFR1	(Timer/Counter 1 Interrupt Flag Register, section 16.11.9)\
			0x37 => TIFR2	(Timer/Counter 2 Interrupt Flag Register, section 18.11.7)\
			   (...)\
			0x3B => PCIFR	(Pin Change Interrupt Flag Register, section 13.2.5)\
			0x3C => EIFR	(External Interrupt Flag Register, section 13.2.2)\
			0x3D => EIMSK	(External Interrupt Mask Register, section 13.2.2)\
			0x3E => GPIOR0	(General Purpose I/O Register 0, section 8.6.6)\
			0x3F => EECR	(EEPROM Control Register, section 8.6.3)\
			0x40 => EEDR	(EEPROM Data Register, section 8.6.2)\
			0x41 => EEARH	(EEPROM Address Register, section 8.6.1)\
			0x42 => EEARL
			0x43 => GTCCR	(General Timer/Counter Control Register, section 17.4.1)\
			0x44 => TCCR0A	(Timer/Counter Control Register A, section 15.9.1)\
			0x45 => TCCR0B	(Timer/Counter Control Register B, section 15.9.2)\
			0x46 => TCNT0	(Timer/Counter Register, section 15.9.3)\
			0x47 => OCR0A	(Output Compare Register A, section 15.9.4)\
			0x48 => OCR0B	(Output Compare Register B, section 15.9.5)\
			   (...)\
			0x4A => GPIOR1	(General Purpose I/O Register 0, section 8.6.5)\
			0x4B => GPIOR2	(General Purpose I/O Register 0, section 8.6.4)\
			0x4C => SPCR	(SPI Control Register, section 19.5.1)\
			0x4D => SPSR	(SPI Status Register, section 19.5.2)\
			0x4E => SPDR	(SPI Data Register, section 19.5.3)\
			   (...)\
			0x50 => ACSR	(Analog Comparator Control and Status Register, Section 23.3.2)\
			   (...)\
			0x53 => SMCR	(Sleep Mode Control Register, Section 10.11.1)\
			0x54 => MCUSR	(MCU Status Register, Section 11.9.1)\
			0x55 => MCUCR	(MCU Control Register, Section 10.11.2)\
			   (...)\
			0x57 => SPMCSR	(Store Program Memory Control and Status Register, section 26.3.1)\
			   (...)\
			0x5D => SPH		<= Stack Pointer (initial value at 0x8FF, RAMEND)\
			0x5E => SPL\
			0x5F => SREG	<= Status Register (With status bits MSB=>LSB [I T H S V N Z C])\
									I = Interrupt Enable\
									T = Bit Copy Storage\
									H = Half Carry flag\
									S = Sign bit\
									V = Two's Complement Overflow flag\
									N = Negative flag\
									Z = Zero flag\
									C = Carry flag\

				
		* Range [0x60,0xFF] = Extended I/O memory

			0x60 => WDTCSR	(Watchdog Timer Control Register, section 11.9.2)\
			0x61 => CLKPR	(Clock Prescale Register, section 9.12.2)\
			   (...)\
			0x64 => PRR		(Power Reduction Register, section 10.11.3)\
			   (...)\
			0x66 => OSCCAL	(Oscillator Calibration Register, section 9.12.1)\
			   (...)\
			0x68 => PCICR	(Pin Change Interrupt Control Register, section 13.2.4)\
			0x69 => EICRA	(External Interrupt Control Register A, section 13.2.1)\
			   (...)\
			0x6B => PCMSK0	(Pin Change Mask Register 0, section 13.2.8)\
			0x6C => PCMSK1	(Pin Change Mask Register 1, section 13.2.7)\
			0x6D => PCMSK2	(Pin Change Mask Register 2, section 13.2.6)\
			0x6E => TIMSK0	(Timer/Counter 0 Interrupt Mask Register, section 15.9.6)\
			0x6F => TIMSK1	(Timer/Counter 1 Interrupt Mask Register, section 16.11.8)\
			0x70 => TIMSK2	(Timer/Counter 2 Interrupt Mask Register, section 18.11.6)\
			   (...)\
			0x78 => ADCL	(ADC Data Register, section 24.9.3)\
			0x79 => ADCH\
			0x7A => ADCSRA	(ADC Control and Status Register A, section 24.9.2)\
			0x7B => ADCSRB	(ADC Control and Status Register B, section 23.3.1)\
			0x7C => ADMUX	(ADC Multiplexer Selection Register, section 24.9.1)\
			   (...)\
			0x7E => DIDR0	(Digital Input Disable Register 0, section 24.9.5)\
			0x7F => DIDR1	(Digital Input Disable Register 1, section 23.3.3)\
			0x80 => TCCR0A	(Timer/Counter 1 Control Register A, section 16.11.1)\
			0x81 => TCCR0B	(Timer/Counter 1 Control Register B, section 16.11.2)\
			0x82 => TCCR1C	(Timer/Counter 1 Control Register C, section 16.11.3)\
			   (...)\
			0x84 => TCNT1L	(Timer/Counter 1, section 16.11.4)\
			0x85 => TCNT1H\
			0x86 => ICR1AL	(Input Capture Register 1, section 16.11.7)\
			0x87 => ICR1AH\
			0x88 => OCR1AL	(Output Compare Register 1 A, section 16.11.5)\
			0x89 => OCR1AH\
			0x8A => OCR1BL	(Output Compare Register 1 B, section 16.11.6)\
			0x8B => OCR1BH\
			   (...)\
			0xB0 => TCCR2A	(Timer/Counter 2 Control Register A, section 18.11.1)\
			0xB1 => TCCR2B	(Timer/Counter 2 Control Register B, section 18.11.2)\
			0xB2 => TCNT2	(Timer/Counter 2 Register, section 18.11.3)\
			0xB3 => OCR2A	(Output Compare Register A, section 18.11.4)\
			0xB4 => OCR2B	(Output Compare Register B, section 18.11.5)\
			   (...)\
			0xB6 => ASSR	(Asynchronous Status Register, section 18.11.8)\
			   (...)\
			0xB8 => TWBR	(TWI Bit Rate Register, section 22.9.1)\
			0xB9 => TWSR	(TWI Status Register, section 22.9.3)\
			0xBA => TWAR	(TWI Address Register, section 22.9.5)\
			0xBB => TWDR	(TWI Data Register, section 22.9.4)\
			0xBC => TWCR	(TWI Control Register, section 22.9.2)\
			0xBD => TWAMR	(TWI (Slave) Address Mask Register, section 22.9.6)\
			   (...)\
			0xC0 => UCSR0A	(USART Control and Status Register 0 A, section 20.11.2)\
			0xC1 => UCSR0B 	(USART Control and Status Register 0 B, section 20.11.2)\
			0xC2 => UCSR0C 	(USART Control and Status Register 0 C, section 20.11.2)\
			   (...)\
			0xC4 => UBRR0L	(USART Baud Rate Register High, section 20.11.5)\
			0xC5 => UBRR0H
			0xC6 => UDR0 	(USART I/O Data Register 0, section 20.11.1)\
			   (...)\


	* 2KB SRAM in the range [0x100, 0x100 + 0x7FF]


# Folder Hierarchy

**Root Directory**

It is the Install Directory, on my machine it's: [arduinodir] = "c:\program files (x86)\Arduino"


You can then find the following subdirectories from there:

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

				« The EEPROM library provides an easy to use interface to interact with the internal
				   non-volatile storage found in AVR based Arduino boards. This library will work on
				   many AVR devices like ATtiny and ATmega chips. »

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
	* They **MUST** have the same name as the project, otherwise VMicro won't recognize it as an arduino project and will attempt to build it as C++
	* They **MUST** contain the setup() function. Otherwise nothing works: no **serial**, no nothing...
		 I guess VMicro attempts to poorly patch the exe assuming the functions have to be there... :/
	* Serial.begin() **MUST** be called in the setup() function in the INO file or it won't fire

* It's easy to revert to a usual C++ project by just creating a "<Project Name>.INO" file with a setup() function
	that calls setup2() defined as an external reference, the setup2() and loop() functions can easily be defined
	in another CPP file somewhere else in the project (I usually create a main.cpp file).

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

