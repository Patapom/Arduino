////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LoRa Commands & Control
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define _SS_MAX_RX_BUFF 512 // Increase buffer size for software serial so we don't risk overwriting the circular buffer!
#include <SoftwareSerial.h>

#define USE_RX_TX_BUFFERS	// Define this to use 2 buffers, 1 for RX, 1 for TX. Undefine to only use a single one (Warning! The ACK methods won't work with a single buffer)

const U32 command_delay_ms = 100;  // Delay between commands

#if !defined(PIN_LORA_RX) || !defined(PIN_LORA_TX)
	#error "You must define the LoRa RX/TX pins!"
#endif

enum SEND_RESULT {
	SR_OK = 0,      // Success!
	SR_INVALID_PAYLOAD_SIZE,
	SR_INVALID_PAYLOAD,
	SR_TIMEOUT,     // Send returned a timeout (command failed to return a response)
	SR_ERROR,       // Send returned an error!

	// Custom result
	SR_NO_ACK,		// Sent but not acknowledged
};

enum RECEIVE_RESULT {
	RR_OK = 0,      // Success!
	RR_EMPTY_BUFFER,// Notifies an empty reception buffer when doing active receive peeking using PeekReceive()
	RR_TIMEOUT,     // Receive returned a timeout (command failed to return a response)
	RR_ERROR,       // Receive returned an error! (error code is returned instead of payload)
};

enum RESPONSE_TYPE {
	RT_OK = 0,      // Got the expected response
	RT_ERROR = 1,   // Wrong response
	RT_TIMEOUT = 2, // The command timed out
};

enum CONFIG_RESULT {
	CR_OK = 0,            // Success!
	CR_INVALID_PARAMETER = 1, // One of the parameters is not in the appropriate range!
	CR_COMMAND_FAILED_ = 2,   // Command failed
	CR_COMMAND_FAILED_AT = 3,             // AT didn't return +OK
	CR_COMMAND_FAILED_AT_NETWORKID = 4,   // AT+NETWORKID didn't return +OK
	CR_COMMAND_FAILED_AT_ADDRESS = 5,     // AT+ADDRESS didn't return +OK
	CR_COMMAND_FAILED_AT_PARAMETER = 6,   // AT+PARAMETER didn't return +OK
	CR_COMMAND_FAILED_AT_CPIN = 7,        // AT+CPIN didn't return +OK
	CR_COMMAND_FAILED_AT_MODE = 8,        // AT+MODE didn't return +OK
	CR_INVALID_PASSWORD = 9,              // INvalid password for AT+CPIN (most likely 0)
};

enum WORKING_MODE {
	WM_TRANSCEIVER = 0, // Transceiver mode (Default)
	WM_SLEEP = 1,       // Sleep mode
	WM_SMART = 2,       // "Smart mode" where the device is in sleep mode for N seconds and in transceiver mode for N' seconds
};

// From section 17 of LORA AT Command documentation
enum LORA_ERROR_CODE {
	LERR_MISSING_EOL = 1,	// There is not “enter” or 0x0D 0x0A in the end of the AT Command.
	LERR_MISSING_AT_HEAD = 2,	// The head of AT command is not “AT” string.
	LERR_UNKNOWN_COMMAND	= 4, 	// Unknown command.
	LERR_DATA_LENGTH_MISMATCH = 5,	// The data to be sent does not match the actual length +ERR=5
	LERR_TX_TIMEOUT = 10,	// TX is over times.+ERR=10
	LERR_CRC_ERROR = 12,	// CRC error. +ERR=12
	LERR_TX_DATA_OVER_240_BYTES = 13,	// TX data exceeds 240bytes. +ERR=13
	LERR_FAILED_TO_WRITE_FLASH = 14,	// Failed to write flash memory. +ERR=14
	LERR_UNKNOWN_FAILURE = 15,	// Unknown failure. +ERR=15
	LERR_TX_NOT_COMPLETED = 17,	// Last TX was not completed +ERR=17
	LERR_INVALID_PREAMBLE_VALUE = 18,	// Preamble value is not allowed. +ERR=18
	LERR_RX_FAILED_HEADER_ERROR = 19,	// RX failed, Header error +ERR=19
	LERR_SMART_MODE_INVALID_TIME = 20,	// The time setting value of the “Smart receiving power saving mode” is not allowed. +ERR=20
};

extern SoftwareSerial	LoRa;	// So others can initialize the LoRa module


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send / Receive
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// _payload, the payload to send via LoRa. The characters can be in [1,255] but MUST NOT CONTAIN '\0'!
SEND_RESULT Send( U16 _targetAddress, const char* _message );
SEND_RESULT Send( U16 _targetAddress, U8 _payloadLength, const char* _payload );

// Wait for a +RCV reply and returns payload info
// NOTE: This is a *blocking* function!
RECEIVE_RESULT ReceiveWait( U16& _targetAddress, U8& _payloadLength, char*& _payload );
RECEIVE_RESULT ReceiveWait( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR );

// Check for a +RCV reply and returns payload info if it's available, or RR_EMPTY_BUFFER if no reply is currently available
// NOTE: This is a *non-blocking* function!
RECEIVE_RESULT ReceivePeek( U16& _targetAddress, U8& _payloadLength, char*& _payload );
RECEIVE_RESULT ReceivePeek( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR );

// Extracts the LoRa reply in the form of "+RCV=<Address>,<Length>,<Data>,<RSSI>,<SNR>"
RECEIVE_RESULT  ExtractReply( char* _reply, U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR );


// Custom functions to send/receive with ACK signal
SEND_RESULT	SendACK( U16 _targetAddress, U8 _payloadLength, const char* _payload, U32 _timeOut_ms, U32 _retriesCount );

RECEIVE_RESULT	ReceiveWaitACK( U16& _targetAddress, U8& _payloadLength, char*& _payload );
RECEIVE_RESULT	ReceiveWaitACK( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR );

RECEIVE_RESULT 	ReceivePeekACK( U16& _targetAddress, U8& _payloadLength, char*& _payload );
RECEIVE_RESULT 	ReceivePeekACK( U16& _targetAddress, U8& _payloadLength, char*& _payload, int& _RSSI, int& _SNR );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CONFIG_RESULT  ConfigureLoRaModule( U8 _networkID, U16 _address );
CONFIG_RESULT  ConfigureLoRaModule( U8 _networkID, U16 _address, U32 _band );
CONFIG_RESULT  ConfigureLoRaModule( U8 _networkID, U16 _address, U32 _band, U8 _spreadingFactor, U8 _bandwidth, U8 _codingRate, U8 _programmedPreamble );

// Sets the working mode for the device (default is WM_TRANSCEIVER)
CONFIG_RESULT  SetWorkingMode( WORKING_MODE _workingMode, U16 _RXTime, U16 _sleepTime );


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Password
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CONFIG_RESULT  SetPassword( U32 _password );

// Apparently, the only way to reset the password is to send an "AT+RESET" command...
//CONFIG_RESULT ClearPassword() {
//  if ( SendCommandAndWaitVerify( "AT+CPIN=00000000\r\n", "+OK" ) != RT_OK ) return CR_COMMAND_FAILED_AT_CPIN;
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


char* WaitReply(); // No timeout
char* WaitReply( U32 _maxIterationsCount );

// NOTE: _command must end with "\r\n"!
void  SendCommand( const char* _command );

// Sends a command and awaits reply
char* SendCommandAndWait( const char* _command );

// Sends a command, waits for the reply and compares to the expected reply
// Return an enum depending on the result
RESPONSE_TYPE  SendCommandAndWaitVerify( const char* _command, const char* _expectedReply );

// For debugging purpose
void  SendCommandAndWaitPrint( const char* _command );