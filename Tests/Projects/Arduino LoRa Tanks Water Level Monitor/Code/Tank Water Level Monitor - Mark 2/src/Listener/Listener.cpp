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

		#if !defined(DEBUG) && !defined(DEBUG_LIGHT)
			delay( 1000 );  // Each cycle is 1000ms
		#else
			delay( 1000 );	// During debug, we can receive a lot of messages during a single second and our payload buffer gets completely messed up!
		#endif
		return;
	};

//	#ifdef DEBUG
//	#ifdef DEBUG_LIGHT
//		Serial.print( str( F("Received payload (%u) = "), U16( payloadLength ) ) );
//		Serial.print( payload );
//	#endif

	// Read measurements
	Measurement		measurements[16];
	Measurement*	measurement = measurements;

	bool			startMeasurement = false;
	Time_ms			now;
	measurements[0].time_s = 0;	// First measurement's time is obviously now

	char*	pLastComma = payload;
	char*	p = payload;
	for ( U8 i=0; i < payloadLength; i++, p++ ) {
		if ( *p == ',' ) {
			// Decode and store a new value
			*p = '\0';
			U16	value = atoi( pLastComma );
			pLastComma = p + 1;	// Skip comma

			if ( startMeasurement ) {
				// The first value of a measurement is the delta time from first measurement
				measurement->time_s = value;
				startMeasurement = false;	// Expecting a value now
			} else {
				// The 2nd value of a measurement is the actual raw time of flight value
				measurement->rawValue_micros = value;
				measurement++;				// One more completed measurement!
				startMeasurement  = true;	// Start a new measurement (expecting a time stamp now)
			}
		}
	}

	// Read last value
	if ( startMeasurement ) {
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

	// Send that to the application on the PC
	Serial.print( str( F("<OK> #%d = "), measurementsCount ) );
	const Measurement&	m0 = measurements[0];
	Serial.print( str( F("%u,%u"), m0.time_s, m0.rawValue_micros ) );
	for ( U16 measurementIndex=1; measurementIndex < measurementsCount; measurementIndex++ ) {
		const Measurement&	m = measurements[measurementIndex];
		Serial.print( str( F(",%u,%u"), m.time_s, m.rawValue_micros ) );
	}
	Serial.println();
}
