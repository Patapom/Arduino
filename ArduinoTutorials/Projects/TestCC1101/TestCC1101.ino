#include "CC1101/CC1101.h"

#define	PIN_GDO0	8
#define	PIN_GDO2	9
#define	PIN_CS		10
#define	PIN_SI		11
#define	PIN_SO		12	// Also GDO1
#define	PIN_CLOCK	13

// the setup function runs once when you press reset or power the board
Pom::CC1101	C( PIN_CS, PIN_CLOCK, PIN_SI, PIN_SO, PIN_GDO0, PIN_GDO2 );

void setup() {
	Serial.begin( 9600 );

//Serial.print( "tmp1 = " );Serial.println( C.tmp1, HEX );
//Serial.print( "tmp2 = " );Serial.println( C.tmp2, HEX );

	Serial.println( "Initializing CC1101" );
	Pom::CC1101::Setup_t	parms;
	C.Init( parms );

	C.SetFrequencyOffset( 0.0f );

	Serial.println( "DONE!" );

	Serial.println( "Dumping register values..." );
	
	byte	initialValues[0x3E];
	C.DumpAllRegisters( initialValues );

	for ( byte i=0; i < 0x3E; i++ ) {
		Serial.print( "0x" );
	 	Serial.print( i, HEX );
		Serial.print( " = 0x" );
	 	Serial.print( initialValues[i], HEX );
		Serial.println();
	}
}

// the loop function runs over and over again until power down or reset
void loop() {
}