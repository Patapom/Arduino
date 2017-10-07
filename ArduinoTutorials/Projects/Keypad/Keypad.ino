static const int	PIN_OFFSET = 2;

void setup() {
	Serial.begin( 9600 );
	for ( int i=0; i < 4; i++ ) {
	    pinMode( PIN_OFFSET + i, INPUT_PULLUP );		// Columns are inputs
	    pinMode( PIN_OFFSET + 4+i, OUTPUT );	// Rows are outputs
		digitalWrite( PIN_OFFSET + 4 + i, LOW );	// Clear rows
	}
	delay( 1000 );
}

void loop() {
	int	counter = 1;
	int	singleKey = 0;
	for ( int Y=0; Y < 4; Y++ ) {
		digitalWrite( PIN_OFFSET + 4 + Y, LOW );
//		delay( 2 );

		int	columns = 0;
		int	bit = 1;
		for ( int X=0; X < 4; X++, bit<<=1, counter++ ) {
			bool	pressed = digitalRead( PIN_OFFSET + X ) == LOW;
			columns |= bit * pressed;
			if ( pressed )
				singleKey = counter;
			delay( 1 );
		}

		digitalWrite( PIN_OFFSET + 4 + Y, HIGH );	// Reset row
//		delay( 2 );

//		Serial.print( " Row[" );
//		Serial.print( Y );
//		Serial.print( "] = " );
//		Serial.println( columns, HEX );
	}
	if ( singleKey > 0 ) {
		Serial.print( "Single Key = " );
		char keyChars[1+16] = { 'D', '#', '0', '*',
								'C', '9', '8', '7',
								'B', '6', '5', '4',
								'A', '3', '2', '1',
							};
		Serial.println( keyChars[singleKey-1] );
	} else {
		Serial.println( "" );
	}

	delay( 50 );
}

