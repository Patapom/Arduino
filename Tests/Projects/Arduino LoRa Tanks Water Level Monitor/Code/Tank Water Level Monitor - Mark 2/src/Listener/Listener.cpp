#include "Listener.h"

void	Listener::setup() {
	pinMode( PIN_LED_RED, OUTPUT );
	pinMode( PIN_LED_GREEN, OUTPUT );
	pinMode( PIN_BUTTON, INPUT );

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

	ERROR( configResult != CR_OK, "LoRa Configuration failed..." );

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

	// Ask for global time
	m_loopTime.GetTime();
	m_clockSetTime.GetTime();
	m_globalTime.year = 2025;
	m_globalTime.day = 139;							// May 19th
	m_globalTime.time.time_ms = 41400ULL * 1000ULL;	// 11:30
	Serial.println( "<CLK?>" );
}

bool	pipou = false;
bool	read_button() { return digitalRead( Listener::PIN_BUTTON ); }
bool	debounce_once() {
	static uint16_t state = 0;
	state = (state<<1) | read_button() | 0xfe00;
	return (state == 0xff00);
}
bool	debounce() {
	for ( int i=0; i < 16; i++ ) {
		if ( debounce_once() )
			return true;
		delay( 10 );
	}
	return false;
}

void	Listener::loop() {

	// Get current time
	m_loopTime.GetTime();
//Log( str(F("%08lX %08lX"), ((U32*) &m_loopTime.time_ms)[1], m_loopTime.time_ms ) );

#if 1
if ( debounce() ) {
	Serial.println( "PIPOU!" );
	pipou = true;
	delay( 500 );
}
#endif

	if ( m_loopTime.time_ms - m_lastUpdateTime.time_ms < 1000ULL ) {
		return;	// Update every second
	}
	m_lastUpdateTime = m_loopTime;

//delay( 1000 );

//LogDebug( str( F("Updating at time %08lX %08lX"), ((U32*) &m_lastUpdateTime.time_ms)[1], m_lastUpdateTime.time_ms ) );
//return;

	// Listen for some command from the PC
	if ( Serial.available() ) {
		ReadCommand();
	}

	// Listen for some payload
	U16		senderAddress;
	U8		payloadLength;
	char*	payload;
	RECEIVE_RESULT	result = ReceivePeekACK( senderAddress, payloadLength, payload );

//<MEASURES> #16 = 136-70|000036B0,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535,56241,65535
//<DEBUG> Received 16 measurements:
//<DEBUG> #0 => 00000174 00
//<DEBUG> #1 => 0000FF03 FFFF
//<DEBUG> #2 => 0000FC94 FFFF
//<DEBUG> #3 => 0000FA24 FFFF
//<DEBUG> #4 => 0000F7B5 FFFF
//<DEBUG> #5 => 0000F546 FFFF
//<DEBUG> #6 => 0000F2D7 FFFF
//<DEBUG> #7 => 0000F068 FFFF
//<DEBUG> #8 => 0000EDF8 FFFF
//<DEBUG> #9 => 0000EB89 FFFF
//<DEBUG> #10 => 0000E91A FFFF
//<DEBUG> #11 => 0000E6AB FFFF
//<DEBUG> #12 => 0000E43B FFFF
//<DEBUG> #13 => 0000E1CC FFFF
//<DEBUG> #14 => 0000DF5D FFFF
//<DEBUG> #15 => 0000DCEE FFFF
//<MEASURES> #21 = 136-70|00005AFA,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535,65531,65535

#if 1
if ( pipou ) {
	pipou = false;
	payload = "1,10,2,20,3,30,4,40,5";
	payloadLength = strlen(payload);
	result = RR_OK;
}
#endif

	if ( result != RR_OK ) {
		// Failed to receive a proper packet
		if ( result == RR_ERROR ) {
			// Extract error code
			LORA_ERROR_CODE	errorCode = LORA_ERROR_CODE( atoi( payload ) );
			LogError( 0, str( F("Receive failed with error code #%d"), U16(errorCode) ) );
			Flash( 50, 10 );  // Error!
		}

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

	U32				localTime_s = m_loopTime.time_ms / 1000;

	bool			readTime = false;		// We're expecting a value, we know the time stamp is 0 (now)
	measurements[0].time_s = localTime_s;	// First measurement's time is obviously now

	char*	pLastComma = payload;
	char*	p = payload;
	for ( U8 i=0; i < payloadLength; i++, p++ ) {
		if ( *p == ',' ) {
			// Decode and store a new value
			*p = '\0';
			U32	value = U32( atoi( pLastComma ) );
			pLastComma = p + 1;	// Skip comma

//LogDebug( str( F("value %d = %08lX"), i, value ) );

			if ( readTime ) {
				// The first value of a measurement is the delta time (in seconds) from the first measurement
				measurement->time_s = S32(localTime_s) - S32(value);
//LogDebug( str( F("value %d => local time %08lX - value %08lX = %08lX"), U16(measurement - measurements), localTime_s, value, measurement->time_s ) );
				readTime = false;	// Expecting a value now
			} else {
				// The 2nd value of a measurement is the actual raw time of flight value
				measurement->rawValue_micros = value;
				measurement++;		// One more completed measurement!
				readTime = true;	// Expecting a time stamp now (start a new measurement)
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
		return;
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
	char	tempBuffer[64];
	int		bufferLength = Serial.readBytesUntil( '\n', tempBuffer, 64 );
	tempBuffer[bufferLength] = '\0';

#ifdef DEBUG_LIGHT
LogDebug( str( F("Received command: %s"), tempBuffer ) );
//Serial.print( tempBuffer )
#endif

	if ( strstr( tempBuffer, str( F("CLK=") ) ) == tempBuffer ) {
		// Set the new clock
		bool	success = ReadDateTime( tempBuffer + 4, bufferLength - 4 );
		Log( str( success ? F("<OK> CLK") : F("<ERROR> CLK") ) );
	} else if ( strstr( tempBuffer, str( F("READ") ) ) == tempBuffer ) {
		// Send the measurements now
		SendMeasurements();
	} else if ( strstr( tempBuffer, str( F("FLUSH") ) ) == tempBuffer ) {
		// Clear the buffer of measurements
		m_measurementsCount = 0;
		Log( str( F("<OK> FLUSH") ) );
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
	U64	totalTime_ms = milliseconds + 1000ULL * (seconds + 60ULL * (minutes + 60ULL * hours));	// A total of 86,400,000 milliseconds per day (31,536,000,000 milliseconds per year, so we definitely need more than a U32! :D)

	m_globalTime.time.time_ms = totalTime_ms;

	// Keep the time at which the clock time was set
	m_clockSetTime.time_ms = m_loopTime.time_ms;

	#ifdef DEBUG_LIGHT
		Log( str( F("Received new clock time:") ) );
		Log( str( F("Date: %d-%d"), m_globalTime.year, m_globalTime.day ) );
		Log( str( F("Time: %02d:%02d:%02d:%03d"), hours, minutes, seconds, milliseconds ) );
	#endif

	return true;
}

// Registers any new measurements to our array of global measurements
// Returns false if no new measurement could be registered
U32	Listener::RegisterMeasurements( LocalMeasurement* _measurements, U32 _measurementsCount ) {

#if 0
//LogDebug( str(F("Global time: %d %d %08lX|%08lX"), m_globalTime.year, m_globalTime.day, ((U32*) &m_globalTime.time.time_ms)[1], ((U32*) &m_globalTime.time.time_ms)[0] ) );
LogDebug( str(F("Received %d measurements:"), _measurementsCount ) );
for ( int i=0; i < _measurementsCount; i++ ) {
//	LogDebug( str(F("#%d => %08lX %04X"), i, _measurements[i].time_s, _measurements[i].rawValue_micros ) );
	LogDebug( str(F("#%d => %ld %04X"), i, _measurements[i].time_s, _measurements[i].rawValue_micros ) );
}
#endif

	U32	existingMeasurementsCount = m_measurementsCount % MEASUREMENTS_COUNT;

	// We just received some measurements with time stamps relative to the start time
	//	• The new measurements are ordered from most recent to oldest
	//	• The first measurement is supposed to be at the current time
	//	• Their time stamps are relative to this device's start time (i.e. 0)
	//
	// We must compare them with the ones we already have and only register the new ones
	//	• The existing measurement are ordered from oldest to most recent
	//	• The last measurement is supposed to be the most recent one
	//	• Their time stamps are relative to this device's start time (i.e. 0)
	//
	LocalMeasurement*	newMeasurement = _measurements;
	U32					newMeasurementIndex = 0;	// Start from the beginning
	for ( ; newMeasurementIndex < _measurementsCount; newMeasurementIndex++, newMeasurement++ ) {

		// Check if this measurement already exists in our list
		bool	alreadyExists = false;
		U32	existingMeasurementIndex = m_measurementsCount - 1;	// Start from the most recent one
		for ( U32 i=0; i < existingMeasurementsCount; i++, existingMeasurementIndex-- ) {
			LocalMeasurement&	existingMeasurement = m_measurements[existingMeasurementIndex % MEASUREMENTS_COUNT];

			if ( newMeasurement->time_s > existingMeasurement.time_s + S32(2*60) ) {
				break;	// No need to check any more of our existing measurements as they'll all be older than 2 minutes from this new measurement...
			}
			if ( existingMeasurement.rawValue_micros != newMeasurement->rawValue_micros ) {
				continue;	// Already differs in value
			}

			// Check if it doesn't differ too much in time...
			U64	absDeltaTime_s = newMeasurement->time_s > existingMeasurement.time_s ? newMeasurement->time_s - existingMeasurement.time_s : existingMeasurement.time_s - newMeasurement->time_s;
			if ( absDeltaTime_s < MEASURE_DELTA_TIME_TOLERANCE_S ) {
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

LogDebug( str(F("Found only %d new measurements"), newMeasurementsCount ) );

	return newMeasurementsCount;
}

// Converts a local time in seconds (i.e. relative to this device's start time) to a global time in seconds (i.e. relative to the clock time)
S32	Listener::ConvertLocal2GlobalTime( S32 _localTime_s ) {
	S32	start2Clock_s = m_clockSetTime.time_ms / 1000;	// Time since the clock was set, relative to the start time
	S32	globalTime_s = _localTime_s - start2Clock_s;	// Correct time on the clock
	return globalTime_s;
}

// Sends our entire array of measurements to the PC via Serial
void	Listener::SendMeasurements() {
	// Format the measurements as a reply
	U32	measurementsCount = min( MEASUREMENTS_COUNT, m_measurementsCount );
	Serial.print( str( F("<MEASURES> #%d = "), measurementsCount ) );

	U32	measurementIndex = (m_measurementsCount - 1) % MEASUREMENTS_COUNT;	// Loop around

	// Write the exact date and time of the first measurement
	const LocalMeasurement&	m0 = m_measurements[measurementIndex];

	S32	firstMeasurementSeconds = ConvertLocal2GlobalTime( m0.time_s );	// Time in seconds since the moment the clock was set
//LogDebug( str(F("m0.time_s = %ld => firstMeasurementSeconds %ld"), m0.time_s, firstMeasurementSeconds ) );

	S16	deltaDays = firstMeasurementSeconds / 86400L;	// 24 * 60 * 60 = 86,400 seconds in a day
	firstMeasurementSeconds -= 86400L * deltaDays;		// Now in [0,86400[
	S16	firstMeasurementDay = S16(m_globalTime.day) + deltaDays;
//LogDebug( str(F("firstMeasurementDay %04X"), firstMeasurementDay ) );

	S16	deltaYears = firstMeasurementDay / 365;			// Problem on bisextile years but who cares? This device won't certainly be active for more than a few months at a time! (might still be a problem though)
	firstMeasurementDay -= 365 * deltaYears;			// Now in [0,365[
	U16	firstMeasurementYear = m_globalTime.year + deltaYears;
//LogDebug( str(F("firstMeasurementYear %04X"), firstMeasurementYear ) );

	Serial.print( str( F("%u-%u|%08lX,%u"), firstMeasurementYear, firstMeasurementDay, firstMeasurementSeconds, m0.rawValue_micros ) );

//LogDebug( str(F("Final values for Delta Year/Day/Seconds %04X/%04X/%08lX"), deltaYears, deltaDays, firstMeasurementSeconds ) );

	// Write the remaining measurements with relative time only
	for ( U16 i=1; i < measurementsCount; i++ ) {
		const LocalMeasurement&	m = m_measurements[(measurementIndex + MEASUREMENTS_COUNT - i) % MEASUREMENTS_COUNT];

		U16	deltaTime_s = m0.time_s - m.time_s;	// Always positive time value
		Serial.print( str( F(",%u,%u"), deltaTime_s, m.rawValue_micros ) );
	}

	Serial.println();

//LogDebug( str(F("Global time: %d %d %08lX|%08lX"), m_globalTime.year, m_globalTime.day, ((U32*) &m_globalTime.time.time_ms)[1], ((U32*) &m_globalTime.time.time_ms)[0] ) );
}
