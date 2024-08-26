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
    message = "OOR";  // Out of range error!
  }

#ifdef DEBUG
Serial.print( timeOfFlight_microSeconds );
Serial.println( " µs" );
#endif

  // Send the response
//  if ( Send( RECEIVER_ADDRESS, message ) != SR_OK ) {
  if ( Reply( _payload, message ) != SR_OK ) {
    Flash( 50, 10 );  // Error!
Serial.println( "Error !" );
  }

  return true;
}

#endif
