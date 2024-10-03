////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines the command handler of the server module
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"

bool  HandleCommand_Runtime( U32 _commandID, U8 _payloadLength, char* _payload ) {
  return true;
}

bool  HandleCommand_Ping( U32 _commandID, U8 _payloadLength, char* _payload ) {
  return true;
}

bool  HandleCommand_Distance( U32 _commandID, U8 _payloadLength, char* _payload ) {
  _payload[_payloadLength] = '\0';  // We shouldn't have any buffer overflow problem here... Payloads are very small
  U32 time_microSeconds = atoi( _payload );
  if ( time_microSeconds > 38000 ) {
    // Out of range!
	Log( str( F("Out of range!") ) );
    return false;
  }

  float distance_meters = ConvertTimeOfFlightToDistance( time_microSeconds );
  Log( str( F("Distance = %f cm"), 100.0f * distance_meters ) );
  return true;
}

// Checks for a known command and handles it, or returns false if the command is not recognized
bool  HandleReply( U16 _senderAddress, U8 _replyLength, char* _reply ) {
	// Ensure reply is correctly formatted:
	//  • 4 characters command name followed by a comma
	//  • A U32 command ID followed by a comma
	//
	if ( _reply[4] != ',' || _reply[9] != ',' )
	return false; // Missing commas
	char*	endPtr;
	U32	commandID = strtol( _reply + 8, &endPtr, 16 );
	ERROR( *endPtr != ',', "Unexpected character!" );
//Log( str( F("Command ID = %d", commandID ) );

	// Check for known commands
	char*	commandName = _reply;
	char*	replyPayload = commandName + 10;
	U8		replyPayloadLength = _replyLength - 10;
	if ( QuickCheckCommand( commandName, str( F("TIME") ) ) ) {
		return HandleCommand_Runtime( commandID, replyPayloadLength, replyPayload );
	} else if ( QuickCheckCommand( commandName, str( F("PING") ) ) ) {
		return HandleCommand_Ping( commandID, replyPayloadLength, replyPayload );
	} else if ( QuickCheckCommand( commandName, str( F("DST0") ) ) ) {
		return HandleCommand_Distance( commandID, replyPayloadLength, replyPayload );
	}

	return false; // Unrecognized command...
}
