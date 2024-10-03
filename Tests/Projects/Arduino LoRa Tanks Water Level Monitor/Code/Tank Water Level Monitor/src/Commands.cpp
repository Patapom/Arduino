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

static char  tempBuffer[256];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Ask for a client to execute the specified command and waits for the reply
//  _timeOutBeforeRetry_ms, indicates the delay before we estimate a command timed out
//  _maxRetriesCount, indicates how many times we should attempt executing the command before giving up
char* ExecuteAndWaitReply( U16 _clientAddress, const char _command[4], U16 _commandID, const char* _payload, U32& _retriesCount, U32 _timeOutBeforeRetry_ms, U32 _maxRetriesCount ) {
	int   payloadLength = strlen( _payload );

	U8    replyLength;
	U16   senderAddress;

	_retriesCount = 0;
	while ( _retriesCount++ < _maxRetriesCount ) {
		// Execute command
		Execute( _clientAddress, _command, _commandID, payloadLength, _payload );

		#ifdef DEBUG_LIGHT
			LogDebug( str( F("Executing command %04X (attempt #%d)"), _commandID, _retriesCount ) );
		#endif

		// Wait for a reply
		U32 waitTimeStart_ms = millis();
		while ( millis() - waitTimeStart_ms < _timeOutBeforeRetry_ms ) {
			delay( 10 );
			char*	reply = NULL;
			RECEIVE_RESULT  RR = ReceivePeek( senderAddress, replyLength, reply );
			if ( RR != RR_OK )
				continue;
			
			// Got a reply! Check for correct command ID
			reply[replyLength] = '\0';  // Make sure we can use the reponse as a regular string

			if ( senderAddress != _clientAddress ) {
				#ifdef DEBUG_LIGHT
					LogDebug( reply );
					LogDebug( str( F("Invalid reply: client ID mismatch!") ) );
				#endif
				reply = NULL;
				continue; // Unexpected sender
			}
			if ( replyLength < 10 ) {
				#ifdef DEBUG_LIGHT
					LogDebug( reply );
					LogDebug( str( F("Invalid reply: reply length too short!") ) );
				#endif
				reply = NULL;
				continue; // Unexpected reply length
			}
			if ( reply[4] != ',' || reply[9] != ',' ) {
				#ifdef DEBUG_LIGHT
					LogDebug( reply );
					LogDebug( str( F("Invalid reply: bad format!") ) );
				#endif
				reply = NULL;
				continue; // Badly formatted reply
			}

			char*	endPtr;
			U16 replyCommandID = strtol( reply + 5, &endPtr, 16 );
			ERROR( *endPtr != ',', "Unexpected character!" );

			if ( replyCommandID != _commandID ) {
				#ifdef DEBUG_LIGHT
					LogDebug( reply );
					LogDebug( str( F("Invalid reply: unexpected command ID %04X!"), replyCommandID ) );
				#endif
				reply = NULL;
				continue;
			}
		
			#ifdef DEBUG_LIGHT
				LogDebug( str( F("Received successful reply %s"), reply ) );
			#endif

			return reply;
		}
	}

	// Failed after too many attempts...
	return NULL;
}

// Ask for a client to execute the specified command and waits for the reply
char* ExecuteAndWaitReply( U16 _clientAddress, const char _command[4], U16 _commandID, const char* _payload, U32& _retriesCount ) {
  	return ExecuteAndWaitReply( _clientAddress, _command,  _commandID, _payload, _retriesCount, 1000, 10 ); // Wait 1s before retrying and retry 10 times before failing
}

// Ask for a client to execute the specified command
SEND_RESULT Execute( U16 _clientAddress, const char _command[4], U16 _commandID, const char* _payload ) {
  return Execute( _clientAddress, _command, _commandID, strlen( _payload ), _payload );
}
SEND_RESULT Execute( U16 _clientAddress, const char _command[4], U16 _commandID, U8 _payloadLength, const char* _payload ) {
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
bool  QuickCheckCommand( const char* _payload, const char _command[4] ) {
	const char*	command = _command;
	if ( *_payload++ != *command++ ) return false;
	if ( *_payload++ != *command++ ) return false;
	if ( *_payload++ != *command++ ) return false;
	if ( *_payload != *command ) return false;
	return true;
}

SEND_RESULT Reply( const char _command[4], U16 _commandID, const char* _reply ) {
  char  commandID[5];
  sprintf( commandID, "%04X", _commandID );
  return Reply( _command, commandID, strlen( _reply ), _reply );
}
SEND_RESULT Reply( const char _command[4], const char _commandID[4], const char* _reply ) {
  return Reply( _command, _commandID, strlen( _reply ), _reply );
}
SEND_RESULT Reply( const char _command[4], U16 _commandID, U8 _replyLength, const char* _reply ) {
  char  commandID[5];
  sprintf( commandID, "%04X", _commandID );
  return Reply( _command, commandID, _replyLength, _reply );
}
SEND_RESULT Reply( const char _command[4], const char _commandID[4], U8 _replyLength, const char* _reply ) {
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
