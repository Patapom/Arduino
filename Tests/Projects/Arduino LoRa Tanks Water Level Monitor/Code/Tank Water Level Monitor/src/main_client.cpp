////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLIENT SIDE
//  • The client side responds to the server's command by executing the commands and sending a response
//    ☼ Typically, a client will host various sensors that can be triggered and/or interrogated on demand by the server
//    ☼ The client-side logic should be minimal: wait for server commands to execute, execute them and send the response...
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"

#ifdef TRANSMITTER

Time_ms	startTime;  // Time at which the loop starts

void setup() {
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
	CONFIG_RESULT configResult = ConfigureLoRaModule( NETWORK_ID, TRANSMITTER );

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

	// Store start time
	startTime.GetTime();
}

U32 runCounter = 0; // How many cycles did we execute?

extern bool  HandleCommand( U8 _payloadLength, const char* _payload );

void  ExecuteCommand_Runtime( U8 _payloadLength, const char* _payload ) {
	Reply( _payload, _payload+5, str( F("%d"), runCounter ) );  // Send back the runtime counter
}

void  ExecuteCommand_Ping(  U8 _payloadLength, const char* _payload ) {
	Reply( _payload, _payload+5, str( F("") ) );  // Just send the ping back...
}

void loop() {
	runCounter++; // 1 more cycle...
	#ifdef DEBUG
		digitalWrite( PIN_LED_GREEN, runCounter & 1 );
	#endif

/* Test SR04 every second
delay( 1000 );
U32 timeOfFlight_microSeconds = MeasureEchoTime( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );
char  message[16];
sprintf( message, "%u µs", U32( timeOfFlight_microSeconds < 38000 ? timeOfFlight_microSeconds : -1 ) );  // -1 means an out of range error!
//LogDebug( str( F("%d µs"), timeOfFlight_microSeconds ) );
LogDebug( message );

return;
*/

	// Check for a command
	U16   senderAddress;
	U8    payloadLength;
	char* payload;
	U8    result;
	//*
	if ( (result = ReceivePeek( senderAddress, payloadLength, payload )) != RR_OK ) {
//  if ( (result = ReceiveWait( senderAddress, payloadLength, payload )) != RR_OK ) {
		// Nothing received...
		#ifdef USE_LOW_POWER_IDLE
			LowPower.idle( SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF );
		#else
			delay( CLIENT_POLL_INTERVAL_MS );
		#endif
		return;
	}
//*/
// Test continuous measurements...
//payload = str( F("CMD=DST0,1234,") );
//payloadLength = strlen( payload );

	#ifdef DEBUG_LIGHT
		Log( str( F("Client 1 => Received %s"), payload ) );
	#endif

	if ( senderAddress != RECEIVER_ADDRESS ) {
		return; // Not from the server...
	}

	if ( payloadLength < 4 ) {
		// Invalid payload!
		Flash( 50, 10 );  // Flash to signal error! => We received something that is badly formatted...
		return;
	}

	// Analyze command
	if ( strstr( payload, str( F("CMD=") ) ) != payload ) {
		// Not a command?
		Flash( 50, 10 );  // Flash to signal error! => We received something that is badly formatted...
		return;
	}

	// Skip "CMD="
	payload += 4;
	payloadLength -= 4;

	if ( payloadLength < 4 ) {
		// Invalid payload!
		Flash( 50, 10 );  // Flash to signal error! => We received something that is badly formatted...
		return;
	}

	// Check for common commands
	if ( QuickCheckCommand( payload, str( F("TIME") ) ) ) {
		ExecuteCommand_Runtime( payloadLength, payload );
		return;
	} else if ( QuickCheckCommand( payload, str( F("PING") ) ) ) {
		ExecuteCommand_Ping( payloadLength, payload );
		return;
	}

	// Let handler check for supported commands
	if ( !HandleCommand( payloadLength, payload ) ) {
		// Unrecognized command?
		Flash( 150, 10 );  // Flash to signal error!
		return;
	}
}

#endif
