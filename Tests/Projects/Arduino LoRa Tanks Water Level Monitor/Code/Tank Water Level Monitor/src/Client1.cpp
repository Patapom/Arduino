////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines the command handler of the client module at address 1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"

#if TRANSMITTER == 1

bool	ExecuteCommand_MeasureDistance( U8 _payloadLength, const char* _payload );

// Returns false if the command couldn't be handled
bool	HandleCommand( U8 _payloadLength, const char* _payload ) {
//_payload[_payloadLength] = '\0';
//Serial.println( _payload );

	if ( QuickCheckCommand( _payload, str( F("DST0") ) ) ) {
		return ExecuteCommand_MeasureDistance( _payloadLength, _payload );
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  "DST0" => measures distance on sensor 0
bool  ExecuteCommand_MeasureDistance( U8 _payloadLength, const char* _payload ) {
	// Measure distance
	U32 timeOfFlight_microSeconds = MeasureEchoTime( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );

	char  message[16];
	sprintf( message, "%u", U16( timeOfFlight_microSeconds < 38000 ? timeOfFlight_microSeconds : -1) );  // -1 means an out of range error!

#ifdef DEBUG
	LogDebug( str( F("%d µs"), timeOfFlight_microSeconds ) );
#endif

#ifdef DEBUG_LIGHT
	Flash( PIN_LED_GREEN, 150, 1 );
	LogDebug( str( F("Client 1 => Replied %s"), message ) );
#endif

	// Send the response
//	if ( Send( RECEIVER_ADDRESS, message ) != SR_OK ) {
	if ( Reply( _payload, _payload+5, message ) != SR_OK ) {
		Flash( 50, 10 );  // Error!
	}

	return true;
}

#endif
