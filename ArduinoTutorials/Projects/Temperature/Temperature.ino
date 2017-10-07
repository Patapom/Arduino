#define PIN	2

void setup() {
	Serial.begin(9600);
}

bool	InRange( int _value, int _expectedValue, int _tolerance ) {
	return abs( _value - _expectedValue ) <= _tolerance;
}

int	GetResults( byte _bitDurations[2*41], float& _humidity, float& _temperature ) {
	// Verify the ACK signal
	if ( !InRange( _bitDurations[0], 80, 20 ) ||
		 !InRange( _bitDurations[1], 80, 20 ) ) {
		return 1;	// Failed to receive ACK signal!
	}

	// Recompose bytes
	byte	values[5] = {0};
	for ( byte bitIndex=0; bitIndex < 40; bitIndex++ ) {
		byte	durationLo = _bitDurations[2*(1+bitIndex)+0];
		byte	durationHi = _bitDurations[2*(1+bitIndex)+1];
		if ( !InRange( durationLo, 50, 15 ) )
			return 2;	// Error in data bit LOW

		bool	bit;
		if ( InRange( durationHi, 20, 15 ) ) {
			bit = false;
		} else if ( InRange( durationHi, 60, 15 ) ) {
			bit = true;
		} else
			return 3;	// Error in data bit HIGH

		values[bitIndex>>3] |= bit ? (0x80 >> (bitIndex & 7)) : 0;
	}

	// Verify checksum
	byte	check = values[0] + values[1] + values[2] + values[3];
	if ( check != values[4] ) {
		Serial.println( values[0], BIN );
		Serial.println( values[1], BIN );
		Serial.println( values[2], BIN );
		Serial.println( values[3], BIN );
		Serial.println( values[4], BIN );
		return 4;	// Checksum error
	}

	// Recompose floating point values
	_humidity = values[0] + values[1] / 255.0f;
	_temperature = values[2] + values[3] / 255.0f;

	return 0;
}

void loop() {

	// Ask for transmit
	pinMode( PIN, OUTPUT );
	digitalWrite( PIN, LOW );
	delay( 20 );
	digitalWrite( PIN, HIGH );
	pinMode( PIN, INPUT );

	// DHT11 starting:
	// Basically we're going to get 41 pulses of various lengths
	//	• The initial pulse will be 80µs LO / 80µs HI and indicates the start of transmission
	//	• The remaining pulses will have 50µs LO and either ~26µs HI for 0 bits, or ~70µs HI for 1 bits
	//
	byte	bitDurations[2*41];

	bool	state = LOW;		// Currently counting LOW states
	byte	bitIndex = 0;
	byte	duration = 0;
	byte	stateDuration = 0;
	byte	delay_us = 10;		// Sample every 10µs

	delayMicroseconds( 10 );	// Important to make sure we're well within the first initial LOW state!

	while ( bitIndex < 82 && duration < 6440 ) {	// Max duration is 41 * 160µs
		bool	value = digitalRead( PIN );
		if ( value != state ) {
			bitDurations[bitIndex++] = stateDuration;
			state = !state;			// Switch expected state
			stateDuration = 0;		// Restart measure
		}

		delayMicroseconds( delay_us );
		duration += delay_us;
		stateDuration += delay_us;
	}

#if 0
	// Write pulse durations
	Serial.println( "Bit durations LO:" );
	for ( int i=0; i <= 40; i++ ) {
		Serial.print( bitDurations[2*i+0] );
		Serial.print( "  " );
	}
	Serial.println( "" );
	Serial.println( "Bit durations HI:" );
	for ( int i=0; i <= 40; i++ ) {
		Serial.print( bitDurations[2*i+1] );
		Serial.print( "  " );
	}
	Serial.println( "Total Duration" );
	Serial.println( duration );
	Serial.println( "" );
#endif

#if 1
	// Get results
	float	humidity, temperature;
	int		errorCode = GetResults( bitDurations, humidity, temperature );
	if ( errorCode != 0 ) {
		Serial.print( "ERROR! Code " );
		Serial.println( errorCode );
	} else {
		Serial.print( "Temperature = " );
		Serial.print( temperature );
		Serial.print( "°C - Humitity = " );
		Serial.print( humidity );
		Serial.println( "%" );
	}
#endif

	// DHT11 sampling rate is 1HZ.
	delay(1000);
}
