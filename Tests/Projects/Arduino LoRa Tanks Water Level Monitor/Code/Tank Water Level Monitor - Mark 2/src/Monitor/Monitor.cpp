#include "Monitor.h"

void	Monitor::setup() {
	// Immediately turn off on board LED
	pinMode( LED_BUILTIN, OUTPUT );
	digitalWrite( LED_BUILTIN, LOW );

	pinMode( PIN_LED_RED, OUTPUT );
	pinMode( PIN_LED_GREEN, OUTPUT );

	// Initiate serial communication
	Serial.begin( 19200 );        // This one is connected to the PC
	while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

	Flash( PIN_LED_GREEN, 50, 10 );

/*  Test LEDS
while ( true ) {
	Flash( PIN_LED_RED, 100, 10 );
	Flash( PIN_LED_GREEN, 100, 10 );
}
//*/

/* Test SR04
while ( true ) {
//	Log( str( F("Allo?") ) );
	float distance = MeasureDistance( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );
	Log( str( F("distance = %d"), distance );
	delay( 100 );
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
	CONFIG_RESULT configResult = ConfigureLoRaModule( NETWORK_ID, LORA_ADDRESS, LORA_CONFIG );

	#ifdef DEBUG_LIGHT
		if ( configResult == CR_OK ) {
			Log( str( F("Configuration successful!") ) );
		} else {
			LogError( 0, str( F("Configuration failed with code %d"), (int) configResult ) );
		}
	#endif

	// Enable "smart mode"
	#if defined(USE_SMART_MODE)
		configResult = SetWorkingMode( WM_SMART, 1000, 10000 ); // Active for 1 second, sleep for 10 seconds
		#ifdef DEBUG_LIGHT
			if ( configResult == CR_OK ) {
				Log( str( F("Smart mode successful!") ) );
			} else {
				LogError( 0, str( F("Smart mode failed with code %d"), (int) configResult ) );
			}
		#endif
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

	// Setup the HC-SR04
	SetupPins_HCSR04( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );
	#ifdef DEBUG_LIGHT
		Log( str( F("HC-SR04 Pins configured.") ) );
	#endif
}

static bool	firstTime = true;
void	Monitor::loop() {

	// Perform a measurement
	#ifdef DEBUG_MONITOR
		U32	sleepDuration_s = 10;	// Sleep for 10 seconds the first time...
	#else
		U32	sleepDuration_s = 60;	// Sleep for 1 minute the first time...
	#endif

	Measurement&	measurement = MeasureDistance();
	if ( !firstTime ) {
		// Compare with previous measurement to determine a flow rate and determine the sleep interval
		// We want to sleep for shorter intervals when the flow is high, and for longer intervals when the flow is low
		Measurement&	previousMeasurement = m_measurements[(m_measurementIndex - 1 + 0xFUL) & 0xFUL];

		// Compute absolute flow rate in litres per minute
		float	currentDistance_m = measurement.GetDistance();
		float	previousDistance_m = previousMeasurement.GetDistance();
		float	delta_m = currentDistance_m - previousDistance_m;
				delta_m = abs( delta_m );	// Only interested in absolute rate of change here
		float	delta_L = delta_m * FULL_TANK_VOLUME_L / FULL_TANK_DISTANCE_M;
		float	delta_s = measurement.time_s - previousMeasurement.time_s;

//Serial.println( previousDistance_m );
//Serial.println( currentDistance_m );
//Serial.println( delta_L );

		ERROR( delta_s < 0.0f, "delta_s cannot be negative!" );	// Must always be positive!
		float	flowRate_LPM = delta_L * 60.0f / delta_s;

		#ifdef DEBUG_LIGHT
			LogDebug( str( F("Previous.time = %u - Current.time = %u"), previousMeasurement.time_s, measurement.time_s ) );
			LogDebug( str( F("Delta_m = %d - Delta_L = %d - Delta_s = %d"), S16(delta_m), S16(delta_L), S16(delta_s) ) );
			Serial.print( F("Flow rate = ") );
			Serial.println( flowRate_LPM );
		#endif

		// Estimate sleep duration based on our range of flow rates
		float	t = (flowRate_LPM - MIN_FLOW_RATE_LPM) / (MAX_FLOW_RATE_LPM - MIN_FLOW_RATE_LPM);
				t = t < 0.0f ? 0.0 : (t > 1.0f ? 1.0f : t);
//Serial.print( "t = " );
//Serial.println( t );
		sleepDuration_s = float(MIN_SLEEP_DURATION_S) + (1.0f - t) * (MAX_SLEEP_DURATION_S - MIN_SLEEP_DURATION_S);	// Sleep longer if flow rate is slow (t=0), for a shorter time if flow rate is fast (t=1)

// Debug => 5s for long sleep, 0s for short sleep
//sleepDuration_s = 5 + t * (0 - 5);
	}

	#ifdef DEBUG_LIGHT
		LogDebug( str( F("Sleeping for %d seconds..."), sleepDuration_s ) );
	#endif

	// Leave a little time to complete tasks
	delay( 500 );

	// Sleep!
	#ifdef USE_LOW_POWER_IDLE
		// Unfortunately, it looks like we can't enter deep sleep for more than 8 seconds?
		U32	sleepCyclesCount = U32( ceil( sleepDuration_s / 8.0f ) );
		#ifdef DEBUG_LIGHT
			LogDebug( str( F("Actually sleeping for %d seconds..."), 8 * sleepCyclesCount ) );
		#endif

		for ( U32 sleepCycleIndex=0; sleepCycleIndex < sleepCyclesCount; sleepCycleIndex++ ) {
			LowPower.powerSave( SLEEP_8S, ADC_OFF, BOD_OFF, TIMER2_OFF );
			m_totalSleepTime_s += 8;	// Account for lost time during sleep!
		}
//		LowPower.idle( SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF );
	#else
		// Wait for required time
		U32	start = millis();
		U32	elapsedTime_s = 0;
		while ( elapsedTime_s < sleepDuration_s ) {
			U32	now = millis();
			elapsedTime_s = (now - start) / 1000;
			delay( 1000 );
		}
	#endif

	firstTime = false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Measurement&	Monitor::MeasureDistance() {

	// Measure distance
	U32	timeOfFlight_microSeconds = ~0UL;
	for ( U32 retriesCount=0; retriesCount < 5; retriesCount++ ) {
		timeOfFlight_microSeconds = MeasureEchoTime( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );

		#ifdef DEBUG
			LogDebug( str( F("%d micros"), timeOfFlight_microSeconds ) );
		#endif

		if ( timeOfFlight_microSeconds < 32000 )
			break;

		delay( 250 );	// Wait a bit before taking another measurement...
	}

	// Store measurement
	Measurement&	measurement = m_measurements[m_measurementIndex++ & 0xFUL];
	measurement.rawValue_micros = timeOfFlight_microSeconds < 32000 ? timeOfFlight_microSeconds : 0xFFFF;  // -1 means an out of range error!

	Time_ms	now;
	U32		now_s = m_totalSleepTime_s + now.GetTime_seconds();
	measurement.time_s = now_s;


//delay( 7000 );
//now.GetTime();
//now_s = now.GetTime_seconds();
//measurement.time_s = U16( now_s );


	#ifdef DEBUG_LIGHT
		LogDebug( str( F("New measurement { %04u, %04u }"), measurement.time_s, measurement.rawValue_micros ) );
	#endif

	// Prepare payload
	// It consists of the measurement value, then the previous measurements with their time (in seconds) adjusted relative to the current time
	// For example, if the last measurement was done 5 minutes ago, then we will send { [(now - last measurement time):16], [measurement value:16] }
	//
	char	payload[240];	// Maximum payload supported by LoRa
	char*	p = payload;
	p += sprintf( p, "%04u", U16( timeOfFlight_microSeconds ) );

	for ( U32 i=1; i < 16; i++ ) {
		Measurement&	previousMeasurement = m_measurements[(m_measurementIndex + 15 - i) & 0xFUL];
		U16				deltaTime_s = U16( now_s ) - previousMeasurement.time_s;
		p += sprintf( p, ",%04u,%04u", U16( deltaTime_s ), U16( previousMeasurement.rawValue_micros ) );
	}

	U32	payloadLength = p - payload;

	#ifdef DEBUG_LIGHT
		Flash( PIN_LED_GREEN, 150, 1 );
		LogDebug( str( F("Monitor => Sending payload size %d"), payloadLength ) );
		Serial.println( payload );
	#endif

	// Send the payload and wait for ACK
	U32	retriesCount = 10;	// Retry 10 times before giving up
	U32	timeOut_ms = 1000;	// Wait for 1s before retrying

	SEND_RESULT	result = SendACK( RECEIVER_ADDRESS, payloadLength, payload, timeOut_ms, retriesCount );


	if ( result != SR_OK ) {
		if ( result == SR_NO_ACK ) {
			LogError( 0, str( F("Payload was sent but not ACK..." ) ) );
		} else {
			LogError( 0, str( F("Failed to send payload! Error code = %u"), U16(result) ) );
		}
		Flash( 50, 10 );  // Error!
	}

	return measurement;
}
