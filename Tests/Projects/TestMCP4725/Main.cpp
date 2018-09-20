#include "Pom/Pom.h"

TWI	twi;

void setup2() {
	twi.SetFrequency( 100000 );

	MCP4725_Begin( twi, 0x60 );	// Device Code = 0x60 | {A2, A1,A0} as LSB.
								// By default, A2 = 0, A1 = 0, A0 = 0.
								// A0 can be dynamically chosen by the user to be GND(0) or VCC(1)
 	MCP4725_SetValueEEPROM( twi, 2048 );	// Default is FULL!
}

void loop() {
	// Write a sine-wave in fast mode (i.e. only 2 bytes required)
	static U32	counter = 0;
	U16			value = (U16) (2047 * (1.0f + cos( (2*PI * counter++) / 512 )));

//value = 4095;

// 				value |= ((BIT_PD0 << 1) | BIT_PD1) << 12;
// 
// SwapBytes( value );
// 
// 	twi.Push( (const U8*) &value, 2 );

//	MCP4725_SetValue( twi, value );

//SerialPrintf( "Pushed value %d\n", value );

	delay( 10 );
}
