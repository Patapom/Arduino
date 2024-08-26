////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines the commands protocol exchanged between server and clients
// Commands must have 4 letters (e.g. "TIME", "DIST", etc.) and are given a unique 16-bits ID that must be returned in the reply
// Typically, the server asks:
//    EXECUTE "TIME",1234
// And the client will reply:
//    REPLY "TIME",1234,<some value for the time>
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"

char  tempBuffer[256];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Ask for a client to execute the specified command
SEND_RESULT Execute( U16 _clientAddress, char _command[4], U16 _commandID, String _payload ) {
  return Execute( _clientAddress, _command, _commandID, _payload.length(), _payload.c_str() );
}
SEND_RESULT Execute( U16 _clientAddress, char _command[4], U16 _commandID, U8 _payloadLength, char* _payload ) {
  // Copy command header
  tempBuffer[0] = 'C';
  tempBuffer[1] = 'M';
  tempBuffer[2] = 'D';
  tempBuffer[3] = '=';

  // Copy command name and comma
  memcpy( tempBuffer + 4, _command, 4 );
  tempBuffer[8] = ',';

  // Copy command ID and comma
  sprintf( tempBuffer + 9, "%04X", _commandID );
  tempBuffer[13] = ',';

  // Copy payload
  memcpy( tempBuffer + 14, _payload, _payloadLength );

  // Send execution request
  return Send( _clientAddress, 14 + _payloadLength, tempBuffer );
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

SEND_RESULT Reply( char _command[4], U16 _commandID, String _reply ) {
  char  commandID[4];
  sprintf( commandID, "%04X", _commandID );
  return Reply( _command, commandID, _reply.length(), _reply.c_str() );
}
SEND_RESULT Reply( char _command[4], char _commandID[4], String _reply ) {
  return Reply( _command, _commandID, _reply.length(), _reply.c_str() );
}
SEND_RESULT Reply( char _command[4], U16 _commandID, U8 _replyLength, char* _reply ) {
  char  commandID[4];
  sprintf( commandID, "%04X", _commandID );
  return Reply( _command, commandID, _replyLength, _reply );
}
SEND_RESULT Reply( char _command[4], char _commandID[4], U8 _replyLength, char* _reply ) {
  // Copy command name and comma
  memcpy( tempBuffer, _command, 4 );
  tempBuffer[4] = ',';

  // Copy command ID and comma
  memcpy( tempBuffer + 5, _commandID, 4 );
  tempBuffer[9] = ',';

  // Copy reply
  memcpy( tempBuffer + 10, _reply, _replyLength );

#ifdef DEBUG
//tempBuffer[5+_replyLength] = '\0';
//Serial.print( "Sending reply " );
//Serial.println( tempBuffer );
#endif

  return Send( RECEIVER_ADDRESS, 10 + _replyLength, tempBuffer );
}
