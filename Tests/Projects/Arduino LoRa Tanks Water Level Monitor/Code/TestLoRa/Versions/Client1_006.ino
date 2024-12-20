////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines the command handler of the client module at address 1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"

#if TRANSMITTER == 1

// Returns false if the command couldn't be handled
bool  HandleCommand( U8 _payloadLength, char* _payload ) {
//_payload[_payloadLength] = '\0';
//Serial.println( _payload );

  if ( QuickCheckCommand( _payload, "DST0" ) ) return ExecuteCommand_MeasureDistance( _payloadLength, _payload );
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  "DST0" => measures distance on sensor 0
bool  ExecuteCommand_MeasureDistance( U8 _payloadLength, char* _payload ) {
  // Measure distance
  U32 timeOfFlight_microSeconds = MeasureEchoTime( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );
  String  message;
  if ( timeOfFlight_microSeconds < 38000 ) {
//    message = "Echo0:";
    message += timeOfFlight_microSeconds;
  } else {
    message = "-1";  // Out of range error!
  }

#ifdef DEBUG
  Serial.print( timeOfFlight_microSeconds );
  Serial.println( " µs" );
#endif

  Flash( PIN_LED_GREEN, 150, 1 );
  #ifdef DEBUG_LIGHT
    Serial.println( String( "Client 1 => Replied " ) + message );
  #endif

  // Send the response
//  if ( Send( RECEIVER_ADDRESS, message ) != SR_OK ) {
  if ( Reply( _payload, _payload+5, message ) != SR_OK ) {
    Flash( 50, 10 );  // Error!
//Serial.println( "Error !" );
  }

  return true;
}

#endif
