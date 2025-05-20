#include "Listener.h"

void	Listener::setup() {
	pinMode( PIN_LED_RED, OUTPUT );
	pinMode( PIN_LED_GREEN, OUTPUT );
	pinMode( PIN_BUTTON, INPUT );

	// Initiate serial communication
	Serial.begin( 19200 );        // This one is connected to the PC
	while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

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

	// Listen for some payload
	U16		senderAddress;
	U8		payloadLength;
	char*	payload;
	RECEIVE_RESULT	result = ReceivePeekACK( senderAddress, payloadLength, payload );

#if 0
if ( pipou ) {
	pipou = false;
	static char		pipouPayload[64];
//	static U32		lastPipouTime_s = 0;
	static U32		pipouCount = 0;

	strcpy( pipouPayload, "1,10,2,21,3,33,4,46,5" );
/*
	char*	temp = pipouPayload;
	temp += sprintf( temp, "%ld", pipouCount );
	U32		now_s = m_loopTime.time_ms / 1000;

	U32		measuresCount = min( 5, m_measurementsCount );
	for ( int i=1; i < measuresCount; i++ ) {	// Build 5 measures each time
		LocalMeasurement&	existingMeasurement = m_measurements[(m_measurementsCount-i) % MEASUREMENTS_COUNT];
		U16					delta_s = now_s - existingMeasurement.time_s;
		temp += sprintf( temp, ",%d,%d", delta_s, existingMeasurement.rawValue_micros );
	}
	*temp++ = '\0';
*/
	pipouCount++;
//	lastPipouTime_s = now_s;

	payload = pipouPayload;
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

//#ifdef DEBUG
#ifdef DEBUG_LIGHT
	Serial.println();
	Serial.print( str( F("Received payload (%u) = "), U16( payloadLength ) ) );
	Serial.println( payload );
#endif

	// Read measurements
	Measurement		measurements[16];
	Measurement*	measurement = measurements;

	bool			readTime = false;	// We're expecting a value, we know the time stamp is 0 (now)
	measurements[0].time_s = 0;			// First measurement's time is obviously now

	char*	pLastComma = payload;
	char*	p = payload;
	for ( U8 i=0; i < payloadLength; i++, p++ ) {
		if ( *p == ',' ) {
			// Decode and store a new value
			*p = '\0';
			U32	value = U32( atoi( pLastComma ) );
			pLastComma = p + 1;	// Skip comma

			if ( readTime ) {
				// The first value of a measurement is the delta time (in seconds) from the first measurement
				measurement->time_s = value;
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
	if ( RegisterMeasurements( m_loopTime, measurements, measurementsCount ) ) {
		// Send that to the application on the PC
		SendMeasurements();
	}
}

// Registers any new measurements to our array of global measurements
// Returns false if no new measurement could be registered
U32	Listener::RegisterMeasurements( const Time_ms& _now, Measurement* _newMeasurements, U32 _newMeasurementsCount ) {

#if 1
//LogDebug( str(F("Global time: %d %d %08lX|%08lX"), m_globalTime.year, m_globalTime.day, ((U32*) &m_globalTime.time.time_ms)[1], ((U32*) &m_globalTime.time.time_ms)[0] ) );
LogDebug( str(F("Received %d measurements:"), _newMeasurementsCount ) );
for ( U32 i=0; i < _newMeasurementsCount; i++ ) {
//	LogDebug( str(F("#%d => %08lX %04X"), i, _measurements[i].time_s, _measurements[i].rawValue_micros ) );
	LogDebug( str(F("#%lu => %d %04X"), i, _newMeasurements[i].time_s, _newMeasurements[i].rawValue_micros ) );
}
#endif

	S32	nowTime_s = _now.time_ms / 1000;
LogDebug( str(F("Now = %ld - Then = %ld"), nowTime_s, m_lastMeasurementTime_s ) );

	// We just received some measurements with time stamps relative to the current time
	//	• The new measurements are ordered from most recent to oldest
	//	• The first measurement is supposed to be at the current time (_now) measured relatively to this device's start time (i.e. 0)
	//	• The next measurements give a delta time (in seconds) relative to the first measurement
	//		(this delta time is always positive although it should be *subtracted* from the current time as these measurements are older)
	//
	// We have the time stamp of the last measurement we received
	//	• We simply accept all measurements more recent than the last measurement
	//
	Measurement*	newMeasurement = _newMeasurements;
	U32				newMeasurementIndex = 0;	// Start from the beginning
	for ( ; newMeasurementIndex < _newMeasurementsCount; newMeasurementIndex++, newMeasurement++ ) {
		S32	newMeasurementTime_s = nowTime_s - S32(newMeasurement->time_s);
		if ( newMeasurementTime_s - m_lastMeasurementTime_s < MEASURE_DELTA_TIME_TOLERANCE_S )
			break;	// This measurement is older than the last received measurement
	}

	// Register only the new measurements
	if ( newMeasurementIndex == 0 ) {
LogDebug( str(F("Nothing new!")) );
		return false;	// No new measurement...
	}

	// Go back one step to reach the last accepted measurement
	newMeasurement--;

	S32	previousMeasurementTime_s = m_lastMeasurementTime_s;
	S32	oldestNewMeasurementTime_s = nowTime_s - S32(newMeasurement->time_s);
	if ( oldestNewMeasurementTime_s - previousMeasurementTime_s > 65535 ) {
		// The delta time between the current and previous batches of measurements is too large to be stored on a U16 so we simply flush the existing measures and start over...
		m_measurementsCount = 0;
		previousMeasurementTime_s = oldestNewMeasurementTime_s;
		LogDebug( str(F("Flushing old data")) );
	}

	for ( U32 i=0; i < newMeasurementIndex; i++, m_measurementsCount++, newMeasurement-- ) {
		Measurement&	targetMeasurement = m_measurements[m_measurementsCount % MEASUREMENTS_COUNT];

		S32	newMeasurementTime_s = nowTime_s - S32(newMeasurement->time_s);

		targetMeasurement.rawValue_micros = newMeasurement->rawValue_micros;
		targetMeasurement.time_s = newMeasurementTime_s - previousMeasurementTime_s;	// Always a positive delta time

		previousMeasurementTime_s = newMeasurementTime_s;

LogDebug( str(F("Registering #%lu = {%d, %04X}"), m_measurementsCount, targetMeasurement.time_s, targetMeasurement.rawValue_micros ) );
	}

	// Store the time of the newest measurement
	m_lastMeasurementTime_s = nowTime_s;

LogDebug( str(F("Found %d new measurements"), newMeasurementIndex ) );

	return newMeasurementIndex;
}

// Sends our entire array of measurements to the PC via Serial
void	Listener::SendMeasurements() {
	if ( m_measurementsCount == 0 )
		return;

	// Format the measurements as a reply
	U32	measurementsCount = min( MEASUREMENTS_COUNT, m_measurementsCount );
	Serial.print( str( F("<MEASURES> #%d = "), measurementsCount ) );

	// Write first measurement without a heading comma
	{
		const Measurement&	measurement = m_measurements[(m_measurementsCount-1) % MEASUREMENTS_COUNT];	// Loop around
		Serial.print( str( F("%u,%u"), measurement.rawValue_micros, measurement.time_s ) );
	}

	// Write the rest
	for ( U32 i=1; i < measurementsCount; i++ ) {
		U32					measurementIndex = (m_measurementsCount-1 - i) % MEASUREMENTS_COUNT;	// Loop around
		const Measurement&	measurement = m_measurements[measurementIndex];
		Serial.print( str( F(",%u,%u"), measurement.rawValue_micros, measurement.time_s ) );
	}

	Serial.println();
}
