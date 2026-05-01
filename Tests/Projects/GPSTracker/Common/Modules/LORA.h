#pragma once

#define DEBUG_ENABLED

#include "../Global.h"
#include <HardwareSerial.h>

// 
// AT+ADDRESS=1        (tracker)
// AT+ADDRESS=2        (base)
// AT+NETWORKID=5
// AT+BAND=433000000   (ou 915 selon module)
// 
// // SF=12 → portée max
// // BW=7 → bande étroite
// // CR=1 → robustesse
// // préambule=4 → standard
// AT+PARAMETER=12,7,1,4
// 
// // TODO → Utiliser SNR et RSSI pour avoir une idée de la distance ?
// 

class LORA {
public:
	static constexpr int	NETWORK_ID = 6;		// All our GPS trackers use network 6 (don't use 5 as it's used by the water monitor!)
//	static constexpr int 	BAND = 433000000;	// 433MHz (long range, ideal for forest and landscapes with obstacles)
	static constexpr int 	BAND = 915000000;	// RYLR998 = 915MHz (faster but shorter range if obstacles)

	enum class	RESPONSE_TYPE {
		INVALID = -1,

		OK = 0,
		ERROR = 1,
		TIME_OUT = 2
	};

	HardwareSerial*	m_serial;
	U16				m_deviceID;
	char			m_strVersion[8];
	
	U8				m_sendBuffer[256];
	char			m_receiveBuffer[256];
	U8				m_receiveBufferLength;	// Size of the received string INCLUDING trailing '\0'!

	RESPONSE_TYPE	m_lastReplyCode;
	char			m_strLastError[256];

public:

	LORA( U16 _deviceID ) {
		m_serial = nullptr;
		m_deviceID = _deviceID;
		m_sendBuffer[0] = '\0';
		m_receiveBuffer[0] = '\0';
		m_receiveBufferLength = 0;
		m_lastReplyCode = RESPONSE_TYPE::INVALID;
	}

	bool	Begin( HardwareSerial& _serial, U32 _baudRate, U8 _pinRX, U8 _pinTX );

	bool	SetMode( bool _sleep );

	// Send payload to target device (use ID=0 to broadcast, i.e. send to all devices)
	void	Send( U16 _targetDeviceID, const char* _payload, U8 _payloadLength );
	void	Sendf( U16 _targetDeviceID, const char* _payload, ... );

	// Tries to read a LORA message
	// Returns the size of the message string or 0 if nothing is received
	U8		Receive();

	const char*	LastErrorString() { return m_strLastError; }	// Returns the last error as a readable string
	const char*	LastReplyCode();								// Returns only the last error code as a readable string

	// Writes the string to the LORA device
	void	Write( const U8* _string, U32 _size );
	void	Write( const U8* _string );
	void	Write( const __FlashStringHelper* _string ) { Write( reinterpret_cast<const U8*>( _string ) ); }
	void	Writef( const __FlashStringHelper* _string, ... );

	// Attempts to read a line from the LORA device
	// Returns the length of the received line, 0 otherwise
	U8		ReadLine();

	// Waits for a specific response from the LORA module
	RESPONSE_TYPE	Expect( const char* _strReply, U32 _timeOut_ms=-1 );
	RESPONSE_TYPE	Expect( const __FlashStringHelper* _strReply, U32 _timeOut_ms=-1 ) { return Expect( (const char*) _strReply, _timeOut_ms ); }

private:
	void	SetLastErrorString( const char* _text, ... );
	void	DebugRead();
	void	DEBUG( const char* _string, ... );
};