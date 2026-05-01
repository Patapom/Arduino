#include "LORA.h"

bool	LORA::Begin( HardwareSerial& _serial, U32 _baudRate, U8 _pinRX, U8 _pinTX ) {

	m_serial = &_serial;
	m_serial->begin( _baudRate, SERIAL_8N1, _pinRX, _pinTX );
	delay( 10 );

	// Check we got the expected response
	Write( F("AT\r\n") );
//DebugRead();
	if ( Expect( F("+OK\r\n"), 1000 ) != RESPONSE_TYPE::OK ) {
		SetLastErrorString( "Received \"%s\" instead of \"+OK\" (REPLY=%s)", m_receiveBuffer, LastReplyCode() );
		return false;
	}

	// Read version number
	Write( F("AT+VER?\r\n") );	// Expecting "+VER=RYLR998_REYAX_V1.2.3"
//DebugRead();
	if ( Expect( F("+VER="), 1000 ) != RESPONSE_TYPE::OK ) {
		SetLastErrorString( "Received \"%s\" instead of \"+VER\" (REPLY=%s)", m_receiveBuffer, LastReplyCode() );
		return false;
	}
	if ( strncmp( m_receiveBuffer+5, "RYLR998_REYAX_V", 15 ) ) {
		SetLastErrorString( "Received bad version number \"%s\" (expected \"RYLR998_REYAX_V\")", m_receiveBuffer+5 );
		return false;
	}
	int	versionLength = m_receiveBufferLength-3 - (5+15);
	if ( versionLength > 7 ) {
		SetLastErrorString( "Version number \"%s\" (%d chars) is too large for our buffer (max 8 characters)!", m_receiveBuffer+5+15, versionLength );
		return false;
	}
	strncpy( m_strVersion, m_receiveBuffer + (5+15), versionLength );
	m_strVersion[versionLength] = '\0';

	// Configure address & network ID
	Writef( F("AT+ADDRESS=%d\r\n"), m_deviceID );
	if ( Expect( F("+OK"), 1000 ) != RESPONSE_TYPE::OK ) {
		SetLastErrorString( "Address → Received \"%s\" instead of \"+OK\" (REPLY=%s)", m_receiveBuffer, LastReplyCode() );
		return false;
	}

	Writef( F("AT+NETWORKID=%d\r\n"), NETWORK_ID );
	if ( Expect( F("+OK"), 1000 ) != RESPONSE_TYPE::OK ) {
		SetLastErrorString( "NetworkID → Received \"%s\" instead of \"+OK\" (REPLY=%s)", m_receiveBuffer, LastReplyCode() );
		return false;
	}

	/////////////////////////////////////////////////////////////////////////
	// Configure for long range
	Writef( F("AT+BAND=%d\r\n"), BAND );
	if ( Expect( F("+OK"), 1000 ) != RESPONSE_TYPE::OK ) {
		SetLastErrorString( "Band → Received \"%s\" instead of \"+OK\" (REPLY=%s)", m_receiveBuffer, LastReplyCode() );
		return false;
	}

	// +22dBm (strongest signal)
	Writef( F("AT+CRFOP=22\r\n") );
	if ( Expect( F("+OK"), 1000 ) != RESPONSE_TYPE::OK ) {
		SetLastErrorString( "Band → Received \"%s\" instead of \"+OK\" (REPLY=%s)", m_receiveBuffer, LastReplyCode() );
		return false;
	}

	// ChatGPT shitty proposal:
	// 	SF=12 → max range
	// 	BW=7 → short band
	// 	CR=1 → robust
	// 	Preamble=4 → Only if network ID == 18 !
	//
//	Write( F("AT+PARAMETER=12,7,1,4\r\n") );

	Write( F("AT+PARAMETER=9,7,1,12\r\n") );	// Default
	if ( Expect( F("+OK"), 1000 ) != RESPONSE_TYPE::OK ) {
		SetLastErrorString( "Parameters → Received \"%s\" instead of \"+OK\" (REPLY=%s)", m_receiveBuffer, LastReplyCode() );
		return false;
	}

	Write( F("AT+MODE=0\r\n") );
	if ( Expect( F("+OK"), 1000 ) != RESPONSE_TYPE::OK ) {
		SetLastErrorString( "Transceiver Mode → Received \"%s\" instead of \"+OK\" (REPLY=%s)", m_receiveBuffer, LastReplyCode() );
		return false;
	}

	return true;
}

bool	LORA::SetMode( bool _sleep ) {
	Writef( F("AT+MODE=%d\r\n"), _sleep ? 1 : 0 );
	if ( Expect( F("+OK"), 1000 ) == RESPONSE_TYPE::OK )
		return true;

	SetLastErrorString( "Transceiver Mode → Received \"%s\" instead of \"+OK\" (REPLY=%s)", m_receiveBuffer, LastReplyCode() );
	return false;
}

void	LORA::Send( U16 _targetDeviceID, const char* _payload, U8 _payloadLength ) {
// Syntax is:
//	AT+SEND=<Address>,<Payload Length>,<Data>
//	<Address>0~65535, When the <Address> is 0,
//	it will send data to all address (From 0 to
//	65535.)
//	<Payload Length> Maximum 240bytes
//	<Data>ASCII Format
//	Example : Send HELLO string to the Address 50,
//	AT+SEND=50,5,HELLO

	if ( _payloadLength > 240 )
		throw "Payload size exceeds maximum value of 240 characters!";

	Writef( F("AT+SEND=%d,%d,%s\r\n"), _targetDeviceID, _payloadLength, _payload );
}

void	LORA::Sendf( U16 _targetDeviceID, const char* _payload, ... ) {
	va_list	arg;
	va_list	copy;
	va_start( arg, _payload );
	va_copy( copy, arg );

	char	payload[256];
	int		payloadLength = vsnprintf( payload, sizeof(payload), (const char*) _payload, copy );

	va_end( copy );

	Send( _targetDeviceID, payload, payloadLength );
}

U8		LORA::Receive() {
	return ReadLine();
//	+RCV=<Address>,<Length>,<Data>,<RSSI>,<SNR>
//	<Address> Transmitter Address ID
//	<Length> Data Length
//	<Data> ASCll Format Data
//	<RSSI> Received Signal Strength Indicator
//	<SNR> Signal-to-noise ratio
}

const char*	LORA::LastReplyCode() {
	switch ( m_lastReplyCode ) {
		case RESPONSE_TYPE::INVALID: return "INVALID";
		case RESPONSE_TYPE::OK: return "OK";
		case RESPONSE_TYPE::ERROR: return "ERROR";
	}

	throw "Unsupported error type!";
}

void	LORA::Write( const U8* _string, U32 _size ) {

#ifdef DEBUG_ENABLED
	// Ensure the string is well formatted as a command!
	if ( _string[_size] != '\0' ) {
		DEBUG( "FATAL ERROR! LORA::Write() → string not terminated by \\0!\r\n" );
		while ( 1 );
	} else if ( _size >= 2 && (_string[_size-2] != '\r' || _string[_size-1] != '\n') ) {
		DEBUG( "ERROR! LORA::Write() → string %s not terminated by <CR><LF>!\r\n", _string );
		while ( 1 );
	}
#endif
//	((char*) _string)[_size] = '\0';	// Just make sure string terminates!

//DEBUG( "Writing \"%s\"\r\n", _string );

	m_serial->write( _string, _size );
}

void	LORA::Write( const U8* _string ) {
	Write( _string, strlen( (const char*) _string ) );
}

void	LORA::Writef( const __FlashStringHelper* _string, ... ) {
	va_list	arg;
	va_list	copy;
	va_start( arg, _string );
	va_copy( copy, arg );

	int		len = vsnprintf( (char*) m_sendBuffer, sizeof(m_sendBuffer), (const char*) _string, copy );

	va_end( copy );

//Debug( (char*) m_sendBuffer );

	Write( m_sendBuffer, len );
}

U8	LORA::ReadLine() {
	if ( !m_serial->available() )
		return 0;

	// Read until EOL
	m_receiveBuffer[0] = '\0';
	m_receiveBufferLength = 0;

//DebugRead();

	char	C = '\0';
	while ( C != '\n' ) {
		C = m_serial->read();
//Serial.print( C );
		m_receiveBuffer[m_receiveBufferLength++] = C;
	}
	m_receiveBuffer[m_receiveBufferLength++] = '\0';

//DEBUG( "Received \"%s\"\r\n", m_receiveBuffer );

	return m_receiveBufferLength - 1;
}

LORA::RESPONSE_TYPE	LORA::Expect( const char* _strReply, U32 _timeOut_ms ) {

//DEBUG( "Expecting \"%s\"\r\n", _strReply );

	U32	start = millis();
	while ( millis() - start < _timeOut_ms ) {
		// Wait for reply
		if ( !ReadLine() ) {
			delay( 1 );
			continue;
		}

		// Compare to expected string
		int	strReplyLength = strlen( (const char*) _strReply );
		int	minLength = strReplyLength < m_receiveBufferLength ? strReplyLength : m_receiveBufferLength;
		return strncmp( (const char*) m_receiveBuffer, (const char*) _strReply, minLength ) == 0 ? RESPONSE_TYPE::OK : RESPONSE_TYPE::ERROR;
	}

	m_lastReplyCode = RESPONSE_TYPE::TIME_OUT;
DEBUG( "TIME OUT!" );

	return m_lastReplyCode;
}

void	LORA::SetLastErrorString( const char* _text, ... ) {
	va_list	arg;
	va_list	copy;
	va_start( arg, _text );
	va_copy( copy, arg );

	int		len = vsnprintf( m_strLastError, sizeof(m_strLastError), (const char*) _text, copy );

	va_end( copy );
}

void	LORA::DebugRead() {
//	m_serial->onReceive( )
//	m_serial->write( "AT+VER?\r\n" );

	// Read response
	char	buffer[256];
	bool	exit = false;
	while ( !exit ) {
		int	charsCount = m_serial->available();
		if ( charsCount == 0 )
			continue;

		m_serial->read( buffer, charsCount );
		for ( int i=0; i < charsCount; i++ ) {
			char	C = buffer[i];
			Serial.print( C );
			if ( C == '\n' )
				exit = true;
		}
	}
}


#ifdef DEBUG_ENABLED
void	LORA::DEBUG( const char* _string, ... ) {

//Serial.printf( "Hého %s\r\n", _string );

	char	loc_buf[256];
	va_list	arg;
	va_list	copy;
	va_start( arg, _string );
	va_copy( copy, arg );
	int		len = vsnprintf( loc_buf, sizeof(loc_buf), (const char*) _string, copy );
	va_end( copy );
	if ( len >= sizeof(loc_buf) ) {
		Serial.println( "DEBUG ERROR → Formatted string is larger than the 256 debug string buffer!" );
		while ( 1 );
	}

	Serial.write( (U8*) loc_buf, len );
}
#else
void	LORA::DEBUG( const char* _string, ... ) {}
#endif