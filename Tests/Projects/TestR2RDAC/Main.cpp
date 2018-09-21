//////////////////////////////////////////////////////////////////////////
// R-2R Resistor Ladder DAC Test
// From Amanda Ghassaei's project (https://www.instructables.com/id/Arduino-Audio-Output/)
//////////////////////////////////////////////////////////////////////////
//
// Check noise reduction LC circuit from section 24.6.1 when plugging something into an analog input

#include "Pom/Pom.h"

void setup2() {
	for ( int i=0; i < 8; i++ )
		pinMode( i, OUTPUT );
}

void loop() {
	static U8	counter = 0;
//	PORTD = counter++;

//	PORTD = U8( 127 + 127 * sinf( counter+=8 ) );

	PORTD = 0;

//	counter++;
//	delay( 10 );
//	delayMicroseconds( 10 );
}
