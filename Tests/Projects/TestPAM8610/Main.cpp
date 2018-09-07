// Check noise reduction LC circuit from section 24.6.1

#include "Pom/Pom.h"

void setup2() {

	pinMode( 8, OUTPUT );
	pinMode( 9, OUTPUT );
}

bool	state = false;
void loop() {
//	Serial.println( "LOOP" );

	delay( 200 );
	digitalWrite( 8, state );
//	digitalWrite( 9, !state );
	state != true;
}
