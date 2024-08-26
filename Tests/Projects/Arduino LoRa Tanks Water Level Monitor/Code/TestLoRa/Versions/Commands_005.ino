////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines the commands protocol exchanged between server and clients
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"

char  tempBuffer[256];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SEND_RESULT Execute( U16 _clientAddress, char _command[4], String _payload ) {
  return Execute( _clientAddress, _command, _payload.length(), _payload.c_str() );
}
SEND_RESULT Execute( U16 _clientAddress, char _command[4], U8 _payloadLength, char* _payload ) {
  // Copy command header
  tempBuffer[0] = 'C';
  tempBuffer[1] = 'M';
  tempBuffer[2] = 'D';
  tempBuffer[3] = '=';

  // Copy command name and comma
  memcpy( tempBuffer + 4, _command, 4 );
  tempBuffer[8] = ',';

  // Copy payload
  memcpy( tempBuffer + 9, _payload, _payloadLength );

  // Send execution request
  return Send( _clientAddress, 9 + _payloadLength, tempBuffer );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLIENT SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Quickly checks for a 4-letters command
bool  QuickCheckCommand( char* _payload, char _command[4] ) {
  char* command = _command;
  if ( *_payload++ != *command++ ) return false;
  if ( *_payload++ != *command++ ) return false;
  if ( *_payload++ != *command++ ) return false;
  if ( *_payload != *command ) return false;
  return true;
}

SEND_RESULT Reply( char _command[4], String _reply ) {
  return Reply( _command, _reply.length(), _reply.c_str() );
}
SEND_RESULT Reply( char _command[4], U8 _replyLength, char* _reply ) {
  memcpy( tempBuffer, _command, 4 );                // Copy command
  tempBuffer[4] = ',';                              // Separate command by a comma
  memcpy( tempBuffer + 5, _reply, _replyLength );   // Copy reply

#ifdef DEBUG
//tempBuffer[5+_replyLength] = '\0';
//Serial.print( "Sending reply " );
//Serial.println( tempBuffer );
#endif

  return Send( RECEIVER_ADDRESS, 4 + 1 + _replyLength, tempBuffer );
}
