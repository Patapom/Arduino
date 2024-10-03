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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Ask for a client to execute the specified command and waits for the reply
//  _timeOutBeforeRetry_ms, indicates the delay before we estimate a command timed out
//  _maxRetriesCount, indicates how many times we should attempt executing the command before giving up
char* ExecuteAndWaitReply( U16 _clientAddress, const char _command[4], U16 _commandID, const char* _payload, U32& _retriesCount, U32 _timeOutBeforeRetry_ms, U32 _maxRetriesCount );
// Ask for a client to execute the specified command and waits for the reply
char* ExecuteAndWaitReply( U16 _clientAddress, const char _command[4], U16 _commandID, const char* _payload, U32& _retriesCount );

// Ask for a client to execute the specified command
SEND_RESULT Execute( U16 _clientAddress, const char _command[4], U16 _commandID, const char* _payload );
SEND_RESULT Execute( U16 _clientAddress, const char _command[4], U16 _commandID, U8 _payloadLength, const char* _payload );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLIENT SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Quickly checks for a 4-letters command
bool  QuickCheckCommand( const char* _payload, const char _command[4] );

SEND_RESULT Reply( const char _command[4], U16 _commandID, const char* _reply );
SEND_RESULT Reply( const char _command[4], const char _commandID[4], const char* _reply );
SEND_RESULT Reply( const char _command[4], U16 _commandID, U8 _replyLength, const char* _reply );
SEND_RESULT Reply( const char _command[4], const char _commandID[4], U8 _replyLength, const char* _reply );
