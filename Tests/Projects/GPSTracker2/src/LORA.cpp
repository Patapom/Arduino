#include "LORA.h"

bool	LORA::Begin( HardwareSerial& _serial, U32 _baudRate, U8 _pinRX, U8 _pinTX ) {

	m_serial = &_serial;
	m_serial->begin( _baudRate, SERIAL_8N1, _pinRX, _pinTX );
	delay( 10 );

	Write( F("AT") );
	return Expect( F("+OK"), 1000 ) == RESPONSE_TYPE::OK;
}

String	LORA::LastErrorString() {
	switch ( m_lastError ) {
		case RESPONSE_TYPE::INVALID: return "INVALID";
	}

	throw "Unsupported error type!";
}


void	LORA::Write( const U8* _string, U32 _size ) {
	m_serial->write( _string, _size );
}
void	LORA::Write( const U8* _string ) {
	Write( _string, strlen( (const char*) _string ) );
}

U8	LORA::ReadLine() {
	if ( !m_serial->available() )
		return 0;

	// Read until EOL
	m_receiveIndex = 0;
	U8*		buffer = m_receiveBuffer;
			buffer[0] = '\0';

	while ( buffer[m_receiveIndex] != '\n' ) {
		buffer[m_receiveIndex++] = m_serial->read();
	}
	buffer[m_receiveIndex++] = '\0';

	return m_receiveIndex - 1;
}

LORA::RESPONSE_TYPE	LORA::Expect( const U8* _strReply, U32 _timeOut_ms ) {

	U32	start = millis();
	while ( millis() - start < _timeOut_ms ) {
		// Wait for reply
		if ( !ReadLine() ) {
			delay( 1 );
			continue;
		}

		// Compare to expected string
		return strncmp( (const char*) m_receiveBuffer, (const char*) _strReply, m_receiveIndex ) == 0 ? RESPONSE_TYPE::OK : RESPONSE_TYPE::ERROR;
	}

	return RESPONSE_TYPE::TIME_OUT;
}

/*void	Bidou() {
//	m_serial->onReceive( )

	m_serial->write( "AT+VER?\r\n" );

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
*/