#include "Arduino.h"
#include "CC1101.h"
// 
// using namespace Pom;
// 
// 
// void	CC1101::SPITransfer( byte _opcode, byte _data ) {
// 	// Enable the line
// 	digitalWrite( m_pin_CS, LOW );
// 
// //	delayMicroseconds( 1 );
// 	delayns( 20 );	// 20ns before transmit
// 
// 	// Now shift out the opcode + data (a 16-bits value)
// 	shiftOut( m_pin_SO, m_pin_Clock, MSBFIRST, _opcode );
// 
// delayns( 55 );	// Min 55ns before data in "Single Access"
// delayns( 76 );	// Min 76ns before data in "Burst Access"
// 
// 	shiftOut( m_pin_SO, m_pin_Clock, MSBFIRST, _data );
// 
// 	delayns( 20 );	// 20ns after transmit
// 
// 	// Latch the data onto the display
// 	digitalWrite( m_pin_CS, HIGH );
// }
// 
