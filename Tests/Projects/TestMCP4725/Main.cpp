//////////////////////////////////////////////////////////////////////////
// Tests the MCP4725 serial DAC
// It turns out that serial transmission is much too slow to accomodate our 16KHz audio target
// Instead, we'll use Amanda Ghassaei's R-2R resistor ladder DAC tp start with so only a single write to PORTD
//	is necessary to obtain a nice conversion. The DAC is 8-bits and a bit subject to noise but I followed her
//	advice and ordered a bunch of TLC7528 that are high-precision dual channel DACs and they'll hopefully
//	be able to meet our needs...
//////////////////////////////////////////////////////////////////////////
//
#include "Pom/Pom.h"

TWI	twi;

void setup2() {
	twi.SetFrequency( 400000 );

	MCP4725_Begin( twi, 0x60 );	// Device Code = 0x60 | {A2, A1,A0} as LSB.
								// By default, A2 = 0, A1 = 0, A0 = 0.
								// A0 can be dynamically chosen by the user to be GND(0) or VCC(1)

//	MCP4725_SetValueEEPROM( twi, 0 );	// Default should be 0!
}

void loop() {

	// Write a sine-wave in fast mode (i.e. only 2 bytes required)
	static U32	counter = 0;
	U16			value = (U16) (2047 * (1.0f + cos( (2*PI * counter++) / 2 )));

	MCP4725_SetValue( twi, value );

//SerialPrintf( "Pushed value %d\n", value );

//	delay( 10 );
}
