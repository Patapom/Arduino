#include "Global.h"

const U32 command_delay_ms = 100;  // Delay between commands

char  LoRaBuffer[256];    // Command/Response buffer
//char  sendPayload[256]; // Send command buffer (command + payload)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send / Receive
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// _payload, the payload to send via LoRa. The characters can be in [1,255] but MUST NOT CONTAIN '\0'!
SEND_RESULT Send( U16 _targetAddress, String _message ) {
  return Send( _targetAddress, _message.length(), _message.c_str() );
}
SEND_RESULT Send( U16 _targetAddress, U8 _payloadLength, char* _payload ) {
  // Check payload
  if ( _payload == NULL  ) return SR_INVALID_PAYLOAD;
  if ( _payloadLength == 0 || _payloadLength > 240 ) return SR_INVALID_PAYLOAD_SIZE;

  // Prepare command
  char* command = LoRaBuffer;
  command += sprintf( command, "AT+SEND=%d,%d,", _targetAddress, _payloadLength );
  memcpy( command, _payload, _payloadLength );           // Copy payload
  command += _payloadLength;
  *command++ = '\r';
  *command++ = '\n';
  *command++ = '\0';

//  if ( SendCommandAndWaitVerify( LoRaBuffer, "+OK" ) != RT_OK ) return SR_ERROR; // <= If using this version then remove the "\r\n" that we add above otherwise the LoRa will believe we send 2 commands and will return an error

  // Use the fast version using char*
#ifdef DEBUG
  Serial.print( "Sending command " );
  Serial.print( LoRaBuffer );
#endif

  SendCommand( LoRaBuffer );
  char* response = WaitResponse();
  if ( response == NULL )
    return SR_TIMEOUT;  // Timeout!

#ifdef DEBUG
  Serial.print( response );
#endif

  if ( strstr( response, "+OK" ) == NULL )
    return SR_ERROR;  // Failed!

  return SR_OK; // Success!
}

// Wait for a +RCV response and returns payload info
// NOTE: This is a *blocking* function!
RECEIVE_RESULT ReceiveWait( U16& _targetAddress, U8& _payloadLength, char*& _payload ) {
  int RSSI, SNR;  // Ignore those values
  return ReceiveWait( _targetAddress, _payloadLength, _payload, RSSI, SNR );
}
RECEIVE_RESULT ReceiveWait( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR ) {
  char* response = WaitResponse();
  if ( response == NULL )
    return RR_TIMEOUT;  // Timeout

  return ExtractResponse( response, _targetAddress, _payloadLength, _payload, _RSSI, _SNR );
}

// Check for a +RCV response and returns payload info if it's available, or RR_EMPTY_BUFFER if no response is currently available
// NOTE: This is a *non-blocking* function!
RECEIVE_RESULT ReceivePeek( U16& _targetAddress, U8& _payloadLength, char*& _payload ) {
  int RSSI, SNR;  // Ignore those values
  return ReceivePeek( _targetAddress, _payloadLength, _payload, RSSI, SNR );
}
RECEIVE_RESULT ReceivePeek( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR ) {
  if ( LoRa.available() == 0 )
    return RR_EMPTY_BUFFER;

  // Read response
  char* p = LoRaBuffer;
  char  C = '\0';
  while ( C != '\n' ) {
    while ( !LoRa.available() );
    C = LoRa.read();
    *p++ = C;
  }
//  *p++ = '\0';  // Terminate string properly so it can be printed

  return ExtractResponse( LoRaBuffer, _targetAddress, _payloadLength, _payload, _RSSI, _SNR );
}

// Extracts the LoRa response in the form of "+RCV=<Address>,<Length>,<Data>,<RSSI>,<SNR>"
RECEIVE_RESULT  ExtractResponse( char* response, U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR ) {
  if ( strstr( response, "+RCV=" ) != response )
    return RR_ERROR;  // Unexpected response!

  // Strip response for address, payload size and payload
  char* p = response + 5; // Skip "+RCV="
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
#ifdef DEBUG
  Serial.print( String( "Address = " ) + String( _targetAddress ) + String( ", payload size = " ) + String( _payloadLength ) );
#endif

  // Strip response for RSSI and SNR
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
  Serial.print( String( ", payload = " ) + _payload );
  Serial.println( String( ", RSSI = " ) + String( _RSSI ) + String( ", SNR = " ) + String( _SNR ) );
#endif

  return RR_OK;
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

  // Send configuration commands
//  SendCommandAndWaitPrint( "AT+IPR?" );
  if ( SendCommandAndWaitVerify( "AT", "+OK" ) != RT_OK ) return CR_COMMAND_FAILED_AT;
  delay( command_delay_ms );
  if ( SendCommandAndWaitVerify( String ( "AT+NETWORKID=" ) + _networkID, "+OK" ) != RT_OK ) return CR_COMMAND_FAILED_AT_NETWORKID;
  delay( command_delay_ms );
  if ( SendCommandAndWaitVerify( String ( "AT+ADDRESS=" ) + _address, "+OK" ) != RT_OK ) return CR_COMMAND_FAILED_AT_ADDRESS;
  delay( command_delay_ms );
  if ( SendCommandAndWaitVerify( String ( "AT+PARAMETER=" ) + _spreadingFactor + "," + _bandwidth + "," + _codingRate + "," + _programmedPreamble, "+OK" ) != RT_OK ) return CR_COMMAND_FAILED_AT_PARAMETER;
  delay( command_delay_ms );

  return CR_OK; // Success!
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Password
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CONFIG_RESULT  SetPassword( U32 _password ) {
  if ( !_password ) return CR_INVALID_PASSWORD;
  if ( SendCommandAndWaitVerify( String ( "AT+CPIN=" ) + String( _password, 16 ), "+OK" ) != RT_OK ) return CR_COMMAND_FAILED_AT_CPIN;
  delay( command_delay_ms );

  return CR_OK; // Success!
}

// Apparently, the only way to reset the password is to send an "AT+RESET" command...
//CONFIG_RESULT ClearPassword() {
//  if ( SendCommandAndWaitVerify( "AT+CPIN=00000000", "+OK" ) != RT_OK ) return CR_COMMAND_FAILED_AT_CPIN;
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


char* WaitResponse() { return WaitResponse( ~0U ); } // No timeout
char* WaitResponse( U32 _maxIterationsCount ) {
  char* p = LoRaBuffer;
  char  receivedChar = '\0';
  int   iterationsCount = 0;
  while ( receivedChar != '\n' && iterationsCount < _maxIterationsCount ) {
    while ( LoRa.available() == 0 ); // Wait until a character is available...
    receivedChar = LoRa.read();
    *p++ = receivedChar;  // Append characters to the received message
    iterationsCount++;
  }
  if ( iterationsCount >= _maxIterationsCount )  
    return  NULL;  // Timeout!
 
  *p++ = '\0';  // Terminate string properly so it can be displayed...

//Serial.println( "Received response!" );
//  Serial.println( LoRaBuffer );  // Print the response to the Serial monitor

  return LoRaBuffer;
}

// NOTE: _command must end with "\r\n"!
void  SendCommand( char* _command ) {
  LoRa.print( _command );
}
void  SendCommand( String _command ) {
  LoRa.print( _command + "\r\n" );
}

// Sends a command and awaits response
char* SendCommandAndWait( String _command ) {
  SendCommand( _command );
  return WaitResponse();
}

// Sends a command, waits for the response and compares to the expected response
// Return an enum depending on the result
RESPONSE_TYPE  SendCommandAndWaitVerify( String _command, String _expectedResponse ) {
#ifdef DEBUG
Serial.print( "Sending command " ); Serial.println( _command );
#endif

  char* response = SendCommandAndWait( _command );
  if  ( response == NULL )
    return RT_TIMEOUT;

#ifdef DEBUG
Serial.print( "Received response " ); Serial.print( response ); // No need to println since the response contains the \r\n...
#endif

  return String( response ).indexOf( _expectedResponse ) == 0 ? RT_OK : RT_ERROR;
}

// For debugging purpose
void  SendCommandAndWaitPrint( String _command ) {
  Serial.println( "Sending command " + _command );
  char* response = SendCommandAndWait( _command );
  if ( response != NULL ) {
    Serial.print( "Received response: " );
    Serial.println( response );  // Print the response to the Serial monitor
  } else {
    Serial.println( "TIMEOUT!" );
  }
}
