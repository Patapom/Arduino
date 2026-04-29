#pragma once

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
#include "Global.h"
#include <HardwareSerial.h>

class LORA {
public:
	enum class	RESPONSE_TYPE {
		INVALID = -1,

		OK = 0,
		ERROR = 1,
		TIME_OUT = 2
	};

	HardwareSerial*	m_serial;
	U8				m_deviceID;
	U8				m_sendBuffer[256];
	U8				m_receiveBuffer[256];
	U8				m_receiveIndex;
	RESPONSE_TYPE	m_lastError;

public:

	LORA( U8 _deviceID ) {
		m_serial = nullptr;
		m_receiveIndex = 0;
		m_lastError = RESPONSE_TYPE::INVALID;
		m_deviceID = _deviceID;
	}

	bool	Begin( HardwareSerial& _serial, U32 _baudRate, U8 _pinRX, U8 _pinTX );

	String	LastErrorString();

	// Writes the string to the LORA device
	void	Write( const U8* _string, U32 _size );
	void	Write( const U8* _string );
	void	Write( const __FlashStringHelper* _string ) { Write( reinterpret_cast<const U8*>( _string ) ); }

	// Attempts to read a line from the LORA device
	// Returns the length of the received line, 0 otherwise
	U8		ReadLine();

	// Waits for a specific response from the LORA module
	RESPONSE_TYPE	Expect( const U8* _strReply, U32 _timeOut_ms=-1 );
	RESPONSE_TYPE	Expect( const __FlashStringHelper* _strReply, U32 _timeOut_ms=-1 ) { return Expect( _strReply, _timeOut_ms ); }
};