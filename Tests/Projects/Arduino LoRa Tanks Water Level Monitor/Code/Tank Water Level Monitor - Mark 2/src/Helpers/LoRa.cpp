﻿#include "Global.h"

static char	LoRaBuffer[_SS_MAX_RX_BUFF];	// Response buffer

SoftwareSerial	LoRa( PIN_LORA_RX, PIN_LORA_TX );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send / Receive
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// _payload, the payload to send via LoRa. The characters can be in [1,255] but MUST NOT CONTAIN '\0'!
SEND_RESULT Send( U16 _targetAddress, const char* _payload ) {
	return Send( _targetAddress, strlen( _payload ), _payload );
}
SEND_RESULT Send( U16 _targetAddress, U8 _payloadLength, const char* _payload ) {
	// Check payload
	if ( _payload == NULL  ) return SR_INVALID_PAYLOAD;
	if ( _payloadLength == 0 || _payloadLength > 240 ) return SR_INVALID_PAYLOAD_SIZE;

	// Prepare command
	LoRa.print( str( F("AT+SEND=%d,%d,"), _targetAddress, _payloadLength ) );
	LoRa.write( _payload, _payloadLength );	// Copy payload
	LoRa.print( "\r\n" );

	char* reply = WaitReply();
	if ( reply == NULL )
		return SR_TIMEOUT;  // Timeout!

#ifdef DEBUG
	LogDebug( str( F("Reply = %s"), reply ) );
#endif

	if ( strstr( reply, str( F("+OK") ) ) == NULL ) {
LogDebug( str( F("Received error: %s"), reply ) );
		return SR_ERROR;  // Failed!
	}

	return SR_OK; // Success!
}

// Wait for a +RCV reply and returns payload info
// NOTE: This is a *blocking* function!
RECEIVE_RESULT ReceiveWait( U16& _targetAddress, U8& _payloadLength, char*& _payload ) {
	int RSSI, SNR;  // Ignore those values
	return ReceiveWait( _targetAddress, _payloadLength, _payload, RSSI, SNR );
}
RECEIVE_RESULT ReceiveWait( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR ) {
	char* reply = WaitReply();
	if ( reply == NULL )
		return RR_TIMEOUT;  // Timeout

	return ExtractReply( reply, _targetAddress, _payloadLength, _payload, _RSSI, _SNR );
}

// Check for a +RCV reply and returns payload info if it's available, or RR_EMPTY_BUFFER if no reply is currently available
// NOTE: This is a *non-blocking* function!
RECEIVE_RESULT ReceivePeek( U16& _targetAddress, U8& _payloadLength, char*& _payload ) {
	int RSSI, SNR;  // Ignore these values
	return ReceivePeek( _targetAddress, _payloadLength, _payload, RSSI, SNR );
}
RECEIVE_RESULT ReceivePeek( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR ) {
	if ( LoRa.available() == 0 )
		return RR_EMPTY_BUFFER;

	// Read reply
	char*	p = LoRaBuffer;
	char	C = '\0';
	while ( C != '\n' ) {
		while ( !LoRa.available() ) { delayMicroseconds( 50 ); }
		C = LoRa.read();
		*p++ = C;
		ERROR( (p - LoRaBuffer) > _SS_MAX_RX_BUFF, "LoRa RX buffer overflow!" );
	}
	*p++ = '\0';  // Terminate string properly so it can be printed

//Serial.print( str( F("Received payload (%d) = "), U16(p-LoRaBuffer) ) );
//Serial.print( LoRaBuffer );

	return ExtractReply( LoRaBuffer, _targetAddress, _payloadLength, _payload, _RSSI, _SNR );
}

// Extracts the LoRa reply in the form of "+RCV=<Address>,<Length>,<Data>,<RSSI>,<SNR>"
RECEIVE_RESULT  ExtractReply( char* _reply, U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR ) {
	_payloadLength = 0;
	_payload = NULL;
	if ( strstr( _reply, str( F("+RCV=") ) ) != _reply ) {
		if ( strstr( _reply, str( F("+ERR=") ) ) ) {
			_reply += 5;	// Skip "+ERR="
			_payload = _reply;
		}
		return RR_ERROR;  // Unexpected reply!
	}

	// Strip reply for address, payload size and payload
	char* p = _reply + 5; // Skip "+RCV="
	char* addressStart = p;
	while ( *p != '\n' && *p != ',' ) { p++; }
	char* addressEnd = p;
	p++;
	char* payloadLengthStart = p;
	while ( *p != '\n' && *p != ',' ) { p++; }
	char* payloadLengthEnd = p;
	p++;
	_payload = p;

	// Extract address and payload length
	*addressEnd = '\0';       // Replace ',' by '\0' so we can convert to integer
	*payloadLengthEnd = '\0';
	_targetAddress = atoi( addressStart );
	_payloadLength = atoi( payloadLengthStart );

	// Strip reply for RSSI and SNR
	char* payloadEnd = _payload + _payloadLength;
	p = payloadEnd;
	p++;
	char* RSSIStart = p;
	while ( *p != '\n' && *p != ',' ) { p++; }
	char* RSSIEnd = p;
	p++;
	char* SNRStart = p;
	while ( *p != '\n' && *p != ',' ) { p++; }
	char* SNREnd = p;

	// Extract RSSI and SNR
	*payloadEnd = '\0';
	*RSSIEnd = '\0'; // Replace ',' by '\0' so we can convert to integer
	*SNREnd = '\0';
	_RSSI = atoi( RSSIStart );
	_SNR = atoi( SNRStart );

#ifdef DEBUG
//  Serial.print( String( F("Address = ") ) + String( _targetAddress ) + String( F(", payload size = ") ) + String( _payloadLength ) );
//  Serial.print( String( F(", payload = ") ) + _payload );
//  Serial.println( String( F(", RSSI = ") ) + String( _RSSI ) + String( F(", SNR = ") ) + String( _SNR ) );
	LogDebug( str( F("Address = %d, payload size = %d, payload = %s, RSSI = %d, SNR = %d"), _targetAddress, _payloadLength, _payload, _RSSI, _SNR )  );
#endif

	return RR_OK;
}

// Sends the command and waits for the ACK signal from the receiver
SEND_RESULT SendWaitACK( U16 _targetAddress, U8 _payloadLength, const char* _payload, U32 _timeOut_ms, U32 _retriesCount ) {

	SEND_RESULT	result = SR_ERROR;
	while ( result != SR_OK && _retriesCount-- > 0 ) {
		result = Send( RECEIVER_ADDRESS, _payloadLength, _payload );
		if ( result != SR_OK ) {
			// Retry until sent...
			delay( 250 );
			continue;
		}

		// Wait for ACK
		result = SR_NO_ACK;

		U16		ackAddress = ~0U;
		U8		replyPayloadLengh = 0;
		char*	replyPayload = NULL;
		RECEIVE_RESULT	receiveResult = RR_EMPTY_BUFFER;
		U32		waitStartTime = millis();
		while ( (receiveResult == RR_EMPTY_BUFFER || ackAddress != _targetAddress) && (millis() - waitStartTime) < _timeOut_ms ) {
			receiveResult = ReceivePeek( ackAddress, replyPayloadLengh, replyPayload );
			delay( 1 );
		}

		if ( receiveResult != RR_OK ) {

//Serial.println( str(F("Retrying (%d) after %d ms"), U16(_retriesCount), U16(millis() - waitStartTime)) );

			continue;	// Retry until ACK received from the target...
		}

#ifdef DEBUG_LIGHT
Serial.print( str(F("Received ack payload (%d) = "), U16(replyPayloadLengh) ) );
Serial.println( replyPayload );
#endif

		if ( strstr( replyPayload, str( F("ACK") ) ) == replyPayload ) {
			result = SR_OK;	// Finally acknowledged!
		}
	}

	return result;
}

// Wait for a message and send and ACK signal when received
RECEIVE_RESULT ReceiveWaitACK( U16& _targetAddress, U8& _payloadLength, char*& _payload ) {
	int RSSI, SNR;  // Ignore those values
	return ReceiveWaitACK( _targetAddress, _payloadLength, _payload, RSSI, SNR );
}
RECEIVE_RESULT ReceiveWaitACK( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR ) {
	char* reply = WaitReply();
	if ( reply == NULL )
		return RR_TIMEOUT;  // Timeout

	// Send back the ACK message
	Send( _targetAddress, str( F("ACK") ) );

	return ExtractReply( reply, _targetAddress, _payloadLength, _payload, _RSSI, _SNR );
}

RECEIVE_RESULT 	ReceivePeekACK( U16& _targetAddress, U8& _payloadLength, char*& _payload ) {
	int	RSSI, SNR ;
	return ReceivePeekACK( _targetAddress, _payloadLength, _payload, RSSI, SNR );
}
RECEIVE_RESULT 	ReceivePeekACK( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR ) {
	// Peek...
	RECEIVE_RESULT	result = ReceivePeek( _targetAddress, _payloadLength, _payload, _RSSI, _SNR );
	if ( result != RR_OK )
		return result;

	// Send back the ACK message
	Send( _targetAddress, str( F("ACK") ) );

	return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CONFIG_RESULT  ConfigureLoRaModule( U8 _networkID, U16 _address ) {
	return ConfigureLoRaModule( _networkID, _address, 915000000 );
}
CONFIG_RESULT  ConfigureLoRaModule( U8 _networkID, U16 _address, U32 _band ) {
	return ConfigureLoRaModule( _networkID, _address, _band, 9, 7, 1, 12 );
}
CONFIG_RESULT  ConfigureLoRaModule( U8 _networkID, U16 _address, U32 _band, U8 _spreadingFactor, U8 _bandwidth, U8 _codingRate, U8 _programmedPreamble ) {
	// Check parameters
	if ( _networkID < 3 || _networkID > 15 ) return CR_INVALID_PARAMETER;
	if ( _spreadingFactor < 5 || _spreadingFactor > 11 ) return CR_INVALID_PARAMETER;
	if ( _bandwidth < 7 || _bandwidth > 9 ) return CR_INVALID_PARAMETER;

//SendCommandAndWaitPrint( str(F("AT+IPR?")) );

	// Send configuration commands
	if ( SendCommandAndWaitVerify( str( F("AT") ), str( F("+OK") ) ) != RT_OK ) return CR_COMMAND_FAILED_AT;
	delay( command_delay_ms );
	if ( SendCommandAndWaitVerify( str( F("AT+NETWORKID=%d"), _networkID ), str( F( "+OK" ) ) ) != RT_OK ) return CR_COMMAND_FAILED_AT_NETWORKID;
	delay( command_delay_ms );
	if ( SendCommandAndWaitVerify( str( F("AT+ADDRESS=%d"), _address ), str( F( "+OK" ) ) ) != RT_OK ) return CR_COMMAND_FAILED_AT_ADDRESS;
	delay( command_delay_ms );
	if ( SendCommandAndWaitVerify( str( F("AT+PARAMETER=%d,%d,%d,%d"), _spreadingFactor, _bandwidth, _codingRate, _programmedPreamble ), str( F("+OK") ) ) != RT_OK ) return CR_COMMAND_FAILED_AT_PARAMETER;
	delay( command_delay_ms );

	return CR_OK; // Success!
}

// Sets the working mode for the device (default is WM_TRANSCEIVER)
CONFIG_RESULT  SetWorkingMode( WORKING_MODE _workingMode, U16 _RXTime, U16 _sleepTime ) {
	if ( _workingMode == WM_SMART ) {
		if ( SendCommandAndWaitVerify( str( F("AT+MODE=2,%d,%d"), _RXTime, _sleepTime ), str( F("+OK") ) ) != RT_OK ) return CR_COMMAND_FAILED_AT_MODE;
	} else {
		if ( SendCommandAndWaitVerify( str( F("AT+MODE=%d"), int(_workingMode) ), str( F("+OK") ) ) != RT_OK ) return CR_COMMAND_FAILED_AT_MODE;
	}
	return CR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Password
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CONFIG_RESULT  SetPassword( U32 _password ) {
	if ( !_password ) return CR_INVALID_PASSWORD;
	if ( SendCommandAndWaitVerify( str( F("AT+CPIN=%08lX"), _password ), str( F("+OK") ) ) != RT_OK ) return CR_COMMAND_FAILED_AT_CPIN;
	delay( command_delay_ms );

	return CR_OK; // Success!
}

// Apparently, the only way to reset the password is to send an "AT+RESET" command...
//CONFIG_RESULT ClearPassword() {
//  if ( SendCommandAndWaitVerify( F("AT+CPIN=00000000"), F("+OK") ) != RT_OK ) return CR_COMMAND_FAILED_AT_CPIN;
//  delay( command_delay_ms );
//  return CR_OK; // Success!
//}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// Commands (from LoRa AT Command.pdf):
//  AT+RESET
//  AT+IPR=<rate>             // Set baud rate (default 57600)
//  AT+ADDRESS=<ID 16 bits>   // Specific to the module (default 0)
//  AT+NETWORKID=[3,15]       // Common to all modules (default 18)
//  AT+BAND=915000000         // Set the center frequency of wireless band. Common to all modules (default 915000000)
//  AT+PARAMETER=9,7,1,12   
//                              [1] <Spreading Factor>: The larger the SF is, the better the sensitivity is. But the transmission time will take longer. 5~11 (default 9) *SF7to SF9 at 125kHz, SF7 to SF10 at 250kHz, and SF7 to SF11 at 500kHz
//                              [2] <Bandwidth>: The smaller the bandwidth is, the better the sensitivity is. But the transmission time will take longer. 7: 125 KHz (default), 8: 250 KHz, 9: 500 KHz
//                              [3] <Coding Rate>: The coding rate will be the fastest if setting it as 1.
//                              [4] <Programmed Preamble>: Preamble code. If the preamble code is bigger, it will result in the less opportunity of losing data.
//                                    Generally preamble code can be set above 10 if under the permission of the transmission time.
//                                    When the Payload length is greater than 100 bytes, recommend to set “AT + PARAMETER = 8,7,1,12”
//  AT+CPIN=<Password>        // Domain password (4 bytes hexa)
//  AT+CRFOP=<power [0,22]>   // RF Output power in dBm (default=22)
//  AT+SEND=<address 16 bits>, <payload size [0,240]>, <payload>  // Due to the program used by the module, the payload part will increase more 8 bytes than the actual data length.

// Waits for a reply from the LoRa module
char* WaitReply( U32 _timeOut_ms, U32 _maxIterationsCount ) {
	char* p = LoRaBuffer;
	char  receivedChar = '\0';
	U32   iterationsCount = 0;
	while ( receivedChar != '\n' ) {
		// Wait until a character is available...
		U32	now_ms = millis();
		while ( !LoRa.available() ) {
			if ( millis() - now_ms > _timeOut_ms )
				break;	// Timeout!
		}
		if ( !LoRa.available() ) {
			iterationsCount++;
			if ( iterationsCount >= _maxIterationsCount )  
				return  NULL;  // Timeout!

			continue;
		}

		receivedChar = LoRa.read();
		*p++ = receivedChar;  // Append characters to the received message
		iterationsCount = 0;

		ERROR( (p - LoRaBuffer) > _SS_MAX_RX_BUFF, "LoRa RX buffer overflow!" );
	}

	*p++ = '\0';  // Terminate string properly so it can be displayed...

//Serial.println( F("Received reply!") );
//Serial.println( LoRaBuffer );  // Print the reply to the Serial monitor

	return LoRaBuffer;
}
char* WaitReply() { return WaitReply( ~0UL, ~0UL ); } // No timeout

// Sends a command and awaits reply
char* SendCommandAndWait( const char* _command, U32 _timeOut_ms, U32 _maxIterationsCount ) {
#ifdef DEBUG
	LogDebug( str( F("Sending command %s"), _command ) );
//Serial.print( F("Sending command ") );
//Serial.println( _command );
#endif

	LoRa.println( _command );
	return WaitReply( _timeOut_ms, _maxIterationsCount );
}
char* SendCommandAndWait( const char* _command ) {
	return SendCommandAndWait( _command, ~0UL, ~0UL );	// No timeout
}

// Sends a command, waits for the reply and compares to the expected reply
// Return an enum depending on the result
RESPONSE_TYPE  SendCommandAndWaitVerify( const char* _command, const char* _expectedReply, U32 _timeOut_ms ) {
	char* reply = SendCommandAndWait( _command, _timeOut_ms, 1 );
	if  ( reply == NULL )
		return RT_TIMEOUT;

#ifdef DEBUG
	LogDebug( str( F("Received reply %s"), reply ) ); // No need to println since the reply contains the \r\n...
#endif

	char* ptrExpectedReply = strstr( reply, _expectedReply );
	return ptrExpectedReply == reply ? RT_OK : RT_ERROR;
}
RESPONSE_TYPE  SendCommandAndWaitVerify( const char* _command, const char* _expectedReply ) {
	return SendCommandAndWaitVerify( _command, _expectedReply, 500 );	// Default time out after 500ms
}

// For debugging purpose
void  SendCommandAndWaitPrint( const char* _command ) {
	Log( str( F("Sending command %s"), _command ) );
	char* reply = SendCommandAndWait( _command );
	if ( reply != NULL ) {
		Log( str( F("Received reply: %s"), reply ) );  // Print the reply to the Serial monitor
	} else {
		LogError( 0, str(F("TIMEOUT!")) );
	}
}
