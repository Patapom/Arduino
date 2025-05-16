#include "Listener.h"

void	Listener::setup() {
	pinMode( PIN_LED_RED, OUTPUT );
	pinMode( PIN_LED_GREEN, OUTPUT );
//	pinMode( PIN_BUTTON, INPUT );

	// Initiate serial communication
	Serial.begin( 19200 );        // This one is connected to the PC
	while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

/*  Test LEDS
while ( true ) {
	Flash( PIN_LED_RED, 100, 10 );
	Flash( PIN_LED_GREEN, 100, 10 );
}
//*/

/*	// Hardware reset LoRa module
	pinMode( PIN_RESET, OUTPUT );
	digitalWrite( PIN_RESET, LOW );
	delay( 200 ); // Advised value is at least 100ms
	digitalWrite( PIN_RESET, HIGH );
*/

	LoRa.begin( LORA_BAUD_RATE ); // This software serial is connected to the LoRa module

/* Software reset takes an annoyingly long time...
	SendCommandAndWaitPrint( str( F("AT+RESET") ) );  // Normally useless due to hard reset above
	delay( 5000 );
//*/

	#ifdef DEBUG_LIGHT
		Log();
		Log( str( F("Initializing...") ) );
	#endif

	// Initialize the LoRa module
	CONFIG_RESULT configResult = ConfigureLoRaModule( NETWORK_ID, RECEIVER_ADDRESS, LORA_CONFIG );

	#ifdef DEBUG_LIGHT
		if ( configResult == CR_OK ) {
			Log( str( F("Configuration successful!") ) );
		} else {
			LogError( 0, str( F("Configuration failed with code %d"), (int) configResult ) );
		}
	#endif

	if ( configResult != CR_OK ) {
		// In error...
		while ( true ) {
			Flash( PIN_LED_RED, 250, 1 );
		}
	}

// Optional password encryption
//	SendCommandAndWaitPrint( str( F("AT+CPIN?") ) );
//	ClearPassword();  Doesn't work! We must reset the chip to properly clear the password...
//	SetPassword( 0x1234ABCDU );
//	SendCommandAndWaitPrint( str( F("AT+CPIN?") ) );

	Flash( PIN_LED_GREEN, 50, 10 );

/* Test LoRa
while ( true ) {
	U16		senderAddress;
	U8		payloadLength;
	char*	payload;
	RECEIVE_RESULT	result = ReceivePeek( senderAddress, payloadLength, payload );
	if ( result == RR_OK ) {
		Send( senderAddress, "ACK" );
		LogDebug( "COUCOU!" );
	} else {
		// Failed to receive a proper packet
		if ( result == RR_ERROR ) {
			// Extract error code
			LORA_ERROR_CODE	errorCode = LORA_ERROR_CODE( atoi( payload ) );
			LogError( 0, str( F("Receive failed with error code #%d"), U16(errorCode) ) );
			Flash( 50, 10 );  // Error!
		}
	}
	delay( 1000 );

//Serial.print( "buffer length = " );
//Serial.println( sizeof(SoftwareSerial::_receive_buffer) );
//Serial.print( "buffer overflow = " );
//Serial.println( LoRa._buffer_overflow ? "yes" : "no" );
//Serial.println( str( F("head = %02X - tail = %02X"), SoftwareSerial::_receive_buffer_head, SoftwareSerial::_receive_buffer_tail ) );
}
//*/

	// Register start time
	m_startTime.GetTime();
	m_clockSetTime.GetTime();

	// Ask for global time
	Serial.println( "<CLK?>" );
}

//bool	read_button() { return digitalRead( PIN_BUTTON ); }
//bool	debounce_once() {
//	static uint16_t state = 0;
//	state = (state<<1) | read_button() | 0xfe00;
//	return (state == 0xff00);
//}
//bool	debounce() {
//	bool	result = false;
//	for ( int i=0; i < 16; i++ ) {
//		result |= debounce_once();
//		delay( 1 );
//	}
//	return result;
//}

void	Listener::loop() {

	// Get current time
	m_loopTime.GetTime();

	// Listen for some command from the PC
	if ( Serial.available() ) {
		ReadCommand();
	}

	// Listen for some payload
	U16		senderAddress;
	U8		payloadLength;
	char*	payload;
	RECEIVE_RESULT	result = ReceivePeekACK( senderAddress, payloadLength, payload );
	if ( result != RR_OK ) {
		// Failed to receive a proper packet
		if ( result == RR_ERROR ) {
			// Extract error code
			LORA_ERROR_CODE	errorCode = LORA_ERROR_CODE( atoi( payload ) );
			LogError( 0, str( F("Receive failed with error code #%d"), U16(errorCode) ) );
			Flash( 50, 10 );  // Error!
		}

		delay( 1000 );  // Each cycle is 1000ms
		return;
	};

//	#ifdef DEBUG
//	#ifdef DEBUG_LIGHT
//		Serial.print( str( F("Received payload (%u) = "), U16( payloadLength ) ) );
//		Serial.print( payload );
//	#endif

	// Read measurements
	LocalMeasurement	measurements[16];
	LocalMeasurement*	measurement = measurements;

	U64				localTime_ms = m_loopTime.time_ms - m_startTime.time_ms;

	bool			readTime = false;		// We're expecting a value, we know the time stamp is 0 (now)
	measurements[0].time_ms = localTime_ms;	// First measurement's time is obviously now

	char*	pLastComma = payload;
	char*	p = payload;
	for ( U8 i=0; i < payloadLength; i++, p++ ) {
		if ( *p == ',' ) {
			// Decode and store a new value
			*p = '\0';
			U16	value = atoi( pLastComma );
			pLastComma = p + 1;	// Skip comma

			if ( readTime ) {
				// The first value of a measurement is the delta time (in seconds) from the first measurement
				measurement->time_ms = localTime_ms - (1000 * value);
				readTime = false;	// Expecting a value now
			} else {
				// The 2nd value of a measurement is the actual raw time of flight value
				measurement->rawValue_micros = value;
				measurement++;				// One more completed measurement!
				readTime  = true;	// Start a new measurement (expecting a time stamp now)
			}
		}
	}

	// Read last value
	if ( readTime ) {
		LogError( 0, str( F("Expecting last received value to be a time of flight value!") ) );
		LogError( 0, str( F("Ignoring entire payload...") ) );
		Flash( 50, 10 );  // Error!
		return;
	}
	if ( (p - pLastComma) == 0 ) {
		LogError( 0, str( F("Empty last value!") ) );
		LogError( 0, str( F("Ignoring entire payload...") ) );
		Flash( 50, 10 );  // Error!
	}

	measurement->rawValue_micros = atoi( pLastComma );
	measurement++;	// Completed!

	U16	measurementsCount = U16( measurement - measurements );

	// Register any new measurement
	if ( RegisterMeasurements( measurements, measurementsCount ) ) {
		// Send that to the application on the PC
		SendMeasurements();
	}
}

void	Listener::ReadCommand() {
	char	tempBuffer[256];
	Serial.readBytesUntil( '\n', tempBuffer, 256 );
	int		bufferLength = strlen( tempBuffer );

	if ( strstr( tempBuffer, str( F("CLK=") ) ) == tempBuffer ) {
		bool	success = ReadDateTime( tempBuffer + 4, bufferLength - 4 );
		Serial.println( success ? F("<OK>") : F("<ERR>") );
	} else if ( strstr( tempBuffer, str( F("READ") ) ) == tempBuffer ) {
		SendMeasurements();
	}
}

// Reads the date and time in the following format "yyyy-ddd|HH:mm:ss:fff" where ddd is the day of year in [0,364]
bool	Listener::ReadDateTime( char* _dateTime, U32 _dateTimeLength ) {
	if ( _dateTimeLength != 21 )
		return false;	// Not the expected length...

	if ( _dateTime[4] != '-'
	  || _dateTime[8] != '|'
	  || _dateTime[11] != ':'
	  || _dateTime[14] != ':'
	  || _dateTime[17] != ':' )
		return false;	// Doesn't contain the expected separators...

	// Replace separators with string terminators
	_dateTime[4] = '\0';
	_dateTime[8] = '\0';
	_dateTime[11] = '\0';
	_dateTime[14] = '\0';
	_dateTime[17] = '\0';

	// Read date
	m_globalTime.year = atoi( _dateTime + 0 );
	m_globalTime.day = atoi( _dateTime + 5 );

	// Read time
	U8	hours = atoi( _dateTime + 9 );
	U8	minutes = atoi( _dateTime + 12 );
	U8	seconds = atoi( _dateTime + 15 );
	U8	milliseconds  = atoi( _dateTime + 18 );
	U32	totalTime_ms = milliseconds + 1000 * (seconds + 60 * (minutes + 60 * hours));	// A total of 86,400,000 milliseconds per day (31,536,000,000 milliseconds per year, so we definitely need more than a U32! :D)

	m_globalTime.time.time_ms = totalTime_ms;

	// Keep the time at which the clock time was set
	m_clockSetTime.time_ms = m_loopTime.time_ms;

	#ifdef DEBUG_LIGHT
		Log( str( F("Received new clock time:") ) );
		Log( str( F("Received new clock time:") ) );
		Log( str( F("Date: %d-%d"), m_globalTime.year, m_globalTime.day ) );
		Log( str( F("Time: %02d:%02d:%02d:%03d"), hours, minutes, seconds, milliseconds ) );
	#endif

	return true;
}

// Registers any new measurements to our array of global measurements
// Returns false if no new measurement could be registered
U32	Listener::RegisterMeasurements( LocalMeasurement* _measurements, U32 _measurementsCount ) {

	U32	existingMeasurementsCount = m_measurementsCount % MEASUREMENTS_COUNT;

	// We just received some measurements with time stamps relative to the start time
	// We must compare them with the ones we already have and only register the new ones
	//
	LocalMeasurement*	newMeasurement = _measurements;
	U32					newMeasurementIndex = 0;	// Start from the beginning
	for ( ; newMeasurementIndex < _measurementsCount; newMeasurementIndex++, newMeasurement++ ) {

		// Check if this measurement already exists in our list
		bool	alreadyExists = false;
		U32	existingMeasurementIndex = m_measurementsCount - 1;
		for ( U32 i=0; i < existingMeasurementsCount; i++, existingMeasurementIndex-- ) {
			LocalMeasurement&	existingMeasurement = m_measurements[existingMeasurementIndex % MEASUREMENTS_COUNT];

			if ( newMeasurement->time_ms > existingMeasurement.time_ms + 2ULL*60ULL*1000ULL ) {
				break;	// No need to check any more of our existing measurements as they'll all be older than 2 minutes from this new measurement...
			}
			if ( existingMeasurement.rawValue_micros != newMeasurement->rawValue_micros ) {
				continue;	// Already differs in value
			}

			// Check if it doesn't differ too much in time...
			U64	absDeltaTime_ms = newMeasurement->time_ms > existingMeasurement.time_ms ? newMeasurement->time_ms - existingMeasurement.time_ms : existingMeasurement.time_ms - newMeasurement->time_ms;
			if ( absDeltaTime_ms < MEASURE_DELTA_TIME_TOLERANCE_MS ) {
				alreadyExists = true;
				break;
			}
		}

		if ( alreadyExists ) {
			break;	// We can stop there as this measurement already exists!
		}
	}

	// Register only the new measurements
	U32	newMeasurementsCount = newMeasurementIndex;
	while ( newMeasurementIndex > 0 ) {
		newMeasurementIndex--;	// We must go in reverse order since the newer measurements are at the beginning of the list here...

		// Copy new measurement
		memcpy( m_measurements + (m_measurementsCount % MEASUREMENTS_COUNT), _measurements + newMeasurementIndex, sizeof(LocalMeasurement) );
		m_measurementsCount++;	// One more new measurement!
	}

	return newMeasurementsCount;
}

// Converts a local time (i.e. relative to the start time) to a global time (i.e. relative to the clock time)
U64		Listener::ConvertLocal2GlobalTime( U64 _localTime_ms ) {
	U64	start2Clock_ms = m_clockSetTime.time_ms - m_startTime.time_ms;	// Time since the clock was set, relative to the start time
	U64	globalTime_ms = _localTime_ms - start2Clock_ms;					// Time since the clock was set
	return globalTime_ms;
}

// Sends our entire array of measurements to the PC via Serial
void	Listener::SendMeasurements() {
	U32	measurementsCount = min( MEASUREMENTS_COUNT, m_measurementsCount );
	Serial.print( str( F("<OK> #%d = "), measurementsCount ) );

	U32	measurementIndex = (m_measurementsCount - measurementsCount) % MEASUREMENTS_COUNT;	// Loop around

	// Write the exact date and time of the first measurement
	const LocalMeasurement&	m0 = m_measurements[measurementIndex];

	DateTime	firstMeasurementDateTime;
	firstMeasurementDateTime.time.time_ms = ConvertLocal2GlobalTime( m0.time_ms );

	U32	deltaDays = firstMeasurementDateTime.time.time_ms / 86400000ULL;	// 24 * 60 * 60 * 1000 = 86,400,000 milliseconds in a day
	firstMeasurementDateTime.time.time_ms -= 86400000ULL * U64(deltaDays);
	firstMeasurementDateTime.day = m_globalTime.day + deltaDays;

	U32	deltaYears = firstMeasurementDateTime.day / 365;	// Problem on bisextile years but who cares? This device won't certainly be active for more than a few months at a time!
	firstMeasurementDateTime.day -= 365 * deltaYears;
	firstMeasurementDateTime.year = m_globalTime.year + deltaYears;

	Serial.print( str( F("%u-%u|%ull,%u"), firstMeasurementDateTime.year, firstMeasurementDateTime.day, firstMeasurementDateTime.time.time_ms, m0.rawValue_micros ) );

	// Write the remaining measurements with relative time only
	for ( U16 i=1; i < measurementsCount; i++ ) {
		const LocalMeasurement&	m = m_measurements[(measurementIndex + MEASUREMENTS_COUNT - i) % MEASUREMENTS_COUNT];

		U32	deltaTime_s = U32( (m0.time_ms - m.time_ms) / 1000 );	// A positive time value
		Serial.print( str( F(",%u,%u"), deltaTime_s, m.rawValue_micros ) );
	}

	Serial.println();
}
