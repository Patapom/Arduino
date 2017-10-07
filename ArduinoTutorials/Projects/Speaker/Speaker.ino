/*
 Name:		Speaker.ino
 Created:	10/7/2017 5:18:45 PM
 Author:	Patapom
*/

#define	PIN_DATA	12
#define	PIN_CLOCK	9
#define	PIN_LATCH	11

#define	DELAY_MS	0//10

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin( 9600 );
	pinMode( PIN_DATA, OUTPUT );
	pinMode( PIN_CLOCK, OUTPUT );
	pinMode( PIN_LATCH, OUTPUT );
}

// the loop function runs over and over again until power down or reset
byte	value = 0xAA;
void loop() {

	// Try parsing received characters
	byte	parsedValue = 0;
	int		parsedValuesCount = 0;
	while ( Serial.available() ) {
		// Try parsing a quartet
		char	bisou = Serial.read();
		byte	quartet = 0;
		bool	charIsOkay = false;
		if ( bisou >= '0' && bisou <= '9' ) {
			quartet = bisou - '0';
			charIsOkay = true;
		} else if ( bisou >= 'A' && bisou <= 'F' ) {
			quartet = 10 + bisou - 'A';
			charIsOkay = true;
		} else if ( bisou >= 'a' && bisou <= 'f' ) {
			quartet = 10 + bisou - 'a';
			charIsOkay = true;
		} else if ( bisou == 'i' ) {
			// Invert!
			value ^= 0xFF;
			break;
		}
		if ( charIsOkay ) {
			// Append quartet to current value
//Serial.print( quartet, HEX );
			parsedValue <<= 4;
			parsedValue |= quartet;
			parsedValuesCount++;	// One more successfully parsed value!
		}
		if ( !charIsOkay || parsedValuesCount == 2 ) {
			// Either we failed or we succeeded parsing the 2 necessary characters...
			Serial.flush();
			break;
		}
	}

	if ( parsedValuesCount == 2 ) {
		value = parsedValue;
		Serial.print( "Received value = " );
		Serial.print( value, HEX );
		Serial.println( );
	}

	// Send the value
	byte	temp = value;
	for ( int bitIndex=0; bitIndex < 8; bitIndex++ ) {
		digitalWrite( PIN_DATA, (temp & 0x80) ? HIGH : LOW );
		digitalWrite( PIN_CLOCK, HIGH );
		delay( DELAY_MS );
		digitalWrite( PIN_CLOCK, LOW );
		delay( DELAY_MS );
		temp <<= 1;
	}
	digitalWrite( PIN_LATCH, HIGH );
	delay( DELAY_MS );
	digitalWrite( PIN_LATCH, LOW );

	delay( 1000 );

//	value ^= 0xFF;
}
