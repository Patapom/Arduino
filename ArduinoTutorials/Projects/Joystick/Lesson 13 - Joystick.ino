// Lesson 13 - 
void setup() {
	Serial.begin( 9600 );
	pinMode( 2, INPUT_PULLUP );
	digitalWrite( 2, HIGH );
}

void loop() {
	int	VX = analogRead( 0 );
	int	VY = analogRead( 1 );
	Serial.print( "VX = " );
	Serial.print( VX );
	Serial.print( " VY = " );
	Serial.print( VY );
	if ( !digitalRead( 2 ) ) {
		Serial.print( " PRESSED!" );
	}
	Serial.println();
}
