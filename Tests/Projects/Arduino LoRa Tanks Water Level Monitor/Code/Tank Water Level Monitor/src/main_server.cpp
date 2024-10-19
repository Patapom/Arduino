////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER SIDE
//  • The server side is located on the local PC, it sends commands to the clients which respond in turn
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"

#ifndef TRANSMITTER

Time_ms	startTime;  // Time at which the loop starts

bool  ExecuteCommand_MeasureDistance( U16 _commandID, bool _forceOutOfRange );

void setup() {
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
	CONFIG_RESULT configResult = ConfigureLoRaModule( NETWORK_ID, RECEIVER_ADDRESS );

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

	// Store start time
	startTime.GetTime();

	Flash( PIN_LED_GREEN, 50, 10 );
}

U32 runCounter = 0; // How many cycles did we execute?

// Represents a single water level measurement at a given time
struct Measurement {
	U32 timeStamp_seconds;    // The time of the measurement
	U32 rawTime_microSeconds; // The raw time of flight from the sensor
};

//Measurement bufferMeasurements[256];  // The ring buffer of 256 measurements (too large!)
Measurement	bufferMeasurements[MAX_MEASUREMENTS];  // The ring buffer of measurements
U32	measurementsCounter = 0;
U32	measurementErrorsCounter = 0;

U32	cyclesCounter = ~0UL;
U32	timeReference_seconds = 0;  // Set by the user. 0 is the date 2024-06-12 at 00:00. A U32 can then count time in seconds up to 136 years...

bool	read_button() { return digitalRead( PIN_BUTTON ); }
bool	debounce_once() {
	static uint16_t state = 0;
	state = (state<<1) | read_button() | 0xfe00;
	return (state == 0xff00);
}
bool	debounce() {
	bool	result = false;
	for ( int i=0; i < 16; i++ ) {
		result |= debounce_once();
		delay( 1 );
	}
	return result;
}

static U16	s_commandID = 0;

void	loop() {

	runCounter++;
	delay( 1000 );  // Each cycle is 1000ms
//	digitalWrite( PIN_LED_GREEN, runCounter & 1 );

	// Ask client to measure a distance every 15 minutes...
	cyclesCounter++;
//	if ( cyclesCounter >= 15 * 60 ) {
	if ( debounce() ) {
		cyclesCounter = 0;  // Reset counter
		ExecuteCommand_MeasureDistance( s_commandID++, false );
	}

	// Notify if client module has been returning too many successive failed measurement commands!
	if ( measurementErrorsCounter == 0 ) {
		digitalWrite( PIN_LED_RED, 0 );	// Clear error state...
	} else if ( measurementErrorsCounter > 4 ) {
		digitalWrite( PIN_LED_RED, runCounter & 2 );  // Blink every 2 seconds
	}

	// Wait for a command from the driving application
	if ( !Serial.available() )
		return;

	char  commandBuffer[32];
	char* command = commandBuffer;
	U16   commandID = 0;  // Invalid ID

	char  C = 1;
	while ( C >= 0 && C != '\n' ) {
		C = Serial.read();
		if ( C == ',' && commandID == 0 ) {
			// Read command ID and restart at the beginning of the buffer to store the actual command name
			*command++ = '\0';  // Terminate string
			commandID = atoi( commandBuffer );
			command = commandBuffer;
		} else {
			*command++ = C;
		}
	}
	if ( C == '\n' )
		command[-1] = '\0';  // Replace '\n' with proper string terminator
	command = commandBuffer;

	#ifdef DEBUG_LIGHT
		LogDebug( str( F("ID %d, \"%s\""), commandID, command ) );
	#endif

	if ( strstr( command, str( F("PING") ) ) == command ) {
		// Perform a simple ping
		U32		retriesCount = 0;
		char*	reply = ExecuteAndWaitReply( 1, str( F("PING") ), commandID, str( F("") ), retriesCount );
		if ( reply != NULL ) {
			LogReply( commandID, str( F("Ping") ) );
		} else {
			LogError( commandID, str( F("Ping failed") ) );
		}
	} else if ( strstr( command, str( F("MEASURE") ) ) == command ) {
		// Perform a measurement
		bool  forceOutOfRange = false;
		#ifdef DEBUG_LIGHT
			if ( strstr( command + 7, str( F("PIPO") ) ) == command+7 ) {
			forceOutOfRange = true;
			}
		#endif

		if ( ExecuteCommand_MeasureDistance( commandID, forceOutOfRange ) ) {
			measurementsCounter--;  // Don't stack it into the buffer
			LogReply( commandID, str( F("TIME=%u"), bufferMeasurements[measurementsCounter & (MAX_MEASUREMENTS-1)].rawTime_microSeconds ) );
		} else {
			LogError( commandID, str( F("Measure failed.") ) );
		}

	} else if ( strstr( command, str( F("SETTIME=") ) ) == command ) {
		// Sets the reference time
		timeReference_seconds = atoi( command + 8 );
		LogReply( commandID, str( F("New reference time set to 0x%08X"), timeReference_seconds ) );

	} else if ( strstr( command, str( F("GETBUFFERSIZE") ) ) == command ) {
		// Return the amount of measurements in the buffer
		LogReply( commandID, str( F("%d"), measurementsCounter < MAX_MEASUREMENTS ? measurementsCounter : MAX_MEASUREMENTS ) );

	} else if ( strstr( command, str( F("READBUFFER") ) ) == command ) {
		// Return the content of the buffer
		U32 count = measurementsCounter < MAX_MEASUREMENTS ? measurementsCounter : MAX_MEASUREMENTS;
		U8  bufferIndex = (measurementsCounter - count) & (MAX_MEASUREMENTS-1); // Index of the first measurement in the ring buffer

		LogReply( commandID, str( F("%d"), count ) );

		U32 checksum = 0;
		for ( U8 i=0; i < count; i++, bufferIndex++ ) {
			Measurement&  m = bufferMeasurements[bufferIndex];
			LogReply( commandID, str( F("%d;%u"), m.timeStamp_seconds, m.rawTime_microSeconds ) );

			checksum += m.timeStamp_seconds;
			checksum += m.rawTime_microSeconds;
		}

		// Send final checksum
		LogReply( commandID, str( F("CHECKSUM=%08X"), checksum ) );

	} else if ( strstr( command, str( F("FLUSH") ) ) == command ) {
		// Flush the buffer
		measurementsCounter = 0;
		measurementErrorsCounter = 0;
		LogReply( commandID, str( F("") ) );
	}
}

////////////////////////////////////////////////////////////////
// Command Execution
//
bool  ExecuteCommand_MeasureDistance( U16 _commandID, bool _forceOutOfRange ) {

	U32	MAX_RETRIES_COUNT = 5;

	U16	rawTime_microSeconds = ~0U;
	U32	retriesCount = 0;
	while ( rawTime_microSeconds == ~0U && retriesCount < 6 * MAX_RETRIES_COUNT ) { // Retry while we're getting an out of range response...
		if ( retriesCount > 0 ) {
			delay( 100 );
		}

		#if 0 // At the moment the sensor is busted so let's just simulate a fake measure command
			char* reply = ExecuteAndWaitReply( 1, F("PING"), _commandID, F("") );

			// Fake reply with a sawtooth signal ...
			if ( reply != NULL ) {
				const int MIN =  1772;  // 1772µs at top tank level
				const int MAX = 11848;  // 11848µs at bottom tank level
//				rawTime_microSeconds = MAX + ((cyclesCounter & 0x3FF) * (MIN - MAX) >> 10);
//				rawTime_microSeconds = MAX + ((cyclesCounter & 0x3F) * (MIN - MAX) >> 6);
				rawTime_microSeconds = U16( MAX + (1.0 + sin( cyclesCounter * (3.1415 / 64.0) )) * float(MIN - MAX) / 2.0);
				reply = str( F("DST0,%04X,%d"), _commandID, rawTime_microSeconds );
			}
		#else
			// Execute the command and wait for the reply
			U32	singleRetriesCount = 0;
//			char* reply = ExecuteAndWaitReply( 1, str( F("DST0") ), _commandID, str( F("") ), singleRetriesCount );
			char* reply = ExecuteAndWaitReply( 1, str( F("DST0") ), _commandID, str( F("") ), singleRetriesCount, SERVER_WAIT_INTERVAL_MS, MAX_RETRIES_COUNT );	// Wait just a bit more than the client's sleep time
			retriesCount += singleRetriesCount;
		#endif

		if ( reply == NULL ) {
			// Command failed after several attempts!
			measurementErrorsCounter++; 	// Count the amount of errors, after too many errors like this we'll consider an issue with the client module!
			Flash( PIN_LED_RED, 150, 10 );  // Notify of a command failure!
LogDebug( str( F("reply == NULL!") ) );
			return false;
		}

		// Read back time measurement
		rawTime_microSeconds = atoi( reply + 10 );
	}

	if ( _forceOutOfRange ) {
		rawTime_microSeconds = ~0U; // Simulate an out of range measurement
	}

LogDebug( str( F("#%04d rawTime_µs = %u (%04X) (retries count %d)"), _commandID, rawTime_microSeconds, rawTime_microSeconds, retriesCount ) );

	// Register a new measurement
	U8  bufferIndex = U8( measurementsCounter ) & (MAX_MEASUREMENTS-1);

	Time_ms now;

	float deltaTime_seconds = now.GetTime_seconds() - startTime.GetTime_seconds();  // Total time since the device is up

	bufferMeasurements[bufferIndex].timeStamp_seconds = timeReference_seconds + U32( deltaTime_seconds );
	bufferMeasurements[bufferIndex].rawTime_microSeconds = rawTime_microSeconds;

	measurementsCounter++;
	measurementErrorsCounter = 0; // Clear errors counter!

	Flash( PIN_LED_GREEN, 250, 1 ); // Notify of a new measurement

	return true;
}

#endif
