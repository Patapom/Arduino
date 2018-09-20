#include "Pom/Pom.h"

#include "TWI.h"

// TODO: Check power-down bits
const bool	BIT_PD0 = 0;
const bool	BIT_PD1 = 0;

TWI	twi;

void setup2() {
	twi.SetFrequency( 100000 );

	twi.BeginTransmit( 0x60 );	// Device Code = 0x60 | {A2, A1,A0} as LSB.
								// By default, A2 = 0, A1 = 0, A0 = 0.
								// A0 can be dynamically chosen by the user to be GND(0) or VCC(1)
}

//extern U32 gs_TWI_InterruptsCounter;

void loop() {
	// Write a sine-wave in fast mode (i.e. only 2 bytes required)
	static U32	counter = 0;
	U16			value = (U16) (2047 * (1.0f + sinf( (2*PI * counter++) / 1024 )));

value = 4095;

				value |= ((BIT_PD0 << 1) | BIT_PD1) << 12;

	twi.Push( (const U8*) &value, 2 );

//SerialPrintf( "Pushed value %d\n", value );

	delay( 100 );
}
