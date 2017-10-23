#include "Arduino.h"
#include "CC1101.h"

using namespace Pom;

void	delayns( uint32_t _nanoseconds ) {
	uint32_t	microseconds = (999 + _nanoseconds) / 1000;	// We don't have enough resolution for nanoseconds here so we round up to the next microsecond
	delayMicroseconds( microseconds );
}

void	CC1101::Init( byte _CS, byte _CLOCK, byte _SI, byte _SO, byte _GD00, byte _GD02 ) {
	// Setup pins
	m_pin_CS = _CS;
	m_pin_Clock = _CLOCK;
	m_pin_SI = _SI;
	m_pin_SO = _SO;
	m_pin_GD00 = _GD00;
	m_pin_GD02 = _GD02;

	pinMode( m_pin_Clock, OUTPUT );
	pinMode( m_pin_CS, OUTPUT );
	pinMode( m_pin_SO, OUTPUT );
	pinMode( m_pin_SI, INPUT );
// 	pinMode( m_pin_GD00, INPUT );
// 	pinMode( m_pin_GD02, INPUT );

	digitalWrite( m_pin_CS, HIGH );	// Clear the line

	// Initialize registers
// 	SPITransfer( OP_SCANLIMIT, 7 );		// Scanlimit is set to max on startup
// 	SPITransfer( OP_DECODEMODE, 0 );	// No decode: we send raw bits!
// 	SPITransfer( OP_INTENSITY, 16 );	// Max intensity!
// 	SPITransfer( OP_DISPLAYTEST, 0 );	// Stop any current test
// 	SPITransfer( OP_SHUTDOWN, 1 );		// Wake up!
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
void	CC1101::SPISingle( bool _write, byte _address, byte _data ) {
	// Enable the line
	digitalWrite( m_pin_CS, LOW );

	delayns( 20 );	// 20ns before transmit

	// Now shift out the opcode + data (a 16-bits value)
	byte	opcode  = (_write ? 0x80 : 0x00)
					| (0*0x40)				// No burst!
					| (_address & 0x3F);
	shiftOut( m_pin_SO, m_pin_Clock, MSBFIRST, opcode );

// 	delayns( 55 );	// Min 55ns before data in "Single Access"
// 	delayns( 76 );	// Min 76ns before data in "Burst Access"
	delayMicroseconds( 1 );

	shiftOut( m_pin_SO, m_pin_Clock, MSBFIRST, _data );

	delayns( 20 );	// 20ns after transmit

	// Latch the data
	digitalWrite( m_pin_CS, HIGH );
}
void	CC1101::SPIBurst( bool _write, byte _address, uint32_t _dataLength, byte* _data ) {
	// Enable the line
	digitalWrite( m_pin_CS, LOW );

	delayns( 20 );	// 20ns before transmit

	// Now shift out the opcode + data (a 16-bits value)
	byte	opcode  = (_write ? 0x80 : 0x00)
					| 0x40					// Burst!
					| (_address & 0x3F);
	shiftOut( m_pin_SO, m_pin_Clock, MSBFIRST, opcode );

// 	delayns( 55 );	// Min 55ns before data in "Single Access"
// 	delayns( 76 );	// Min 76ns before data in "Burst Access"
	delayMicroseconds( 1 );

	while ( _dataLength-- ) {
		shiftOut( m_pin_SO, m_pin_Clock, MSBFIRST, *_data++ );
	}

	delayns( 20 );	// 20ns after transmit

	// Latch the data
	digitalWrite( m_pin_CS, HIGH );
}

void	CC1101::SetPATable( byte _powerTable[8] ) {
	SPIBurst( true, 0x3E, 8, _powerTable );
}
