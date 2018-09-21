//////////////////////////////////////////////////////////////////////////
// R-2R Resistor Ladder DAC Test
// From Amanda Ghassaei's project (https://www.instructables.com/id/Arduino-Audio-Output/)
//////////////////////////////////////////////////////////////////////////
//
// Check noise reduction LC circuit from section 24.6.1 when plugging something into an analog input

#include "Pom/Pom.h"

U8	sineWave[256];

#if 1
//////////////////////////////////////////////////////////////////////////
// User timer1 for generation
//
void setup2() {
	cli();	// Clear interrupt flag

	for ( int i=0; i < 8; i++ )
		pinMode( i, OUTPUT );
	for ( int i=0; i < 256; i++ )
		sineWave[i] = U8( 127 + 127 * sinf( 2.0f * PI * i / 255 ) );


	Timer1::Init( Timer1::Clk1, Timer1::CTC_OCR1A );	// Clear Timer and Compare Match => Will interrupt when counter == OCR1A
	Timer1::SetOutputCompareA_CTCFrequency( 32.0f );	// Interrupt at 32KHz
	Timer1::EnableInterrupts( true );

	sei();
}

ISR( TIMER1_COMPA_vect ) {
	static U8	counter = 0;
	PORTD = sineWave[counter++];
}

void loop() {
	static int	c = 0;
	static int	inc = 1;

	float	f = 2.0f * PI * c / 255;
	for ( int i=0; i < 256; i++ )
		sineWave[i] = U8( 127 + 127 * sinf( f * i ) );

	c += inc;
	if ( c == 0 )
		inc = 1;
	else if ( c == 40 )
		inc =-1;
}


#else
//////////////////////////////////////////////////////////////////////////
// Use main loop for generation
void setup2() {
	for ( int i=0; i < 8; i++ )
		pinMode( i, OUTPUT );
	for ( int i=0; i < 256; i++ )
		sineWave[i] = U8( 127 + 127 * sinf( 2.0f * PI * i / 255 ) );
}

void loop() {
	static U8	counter = 0;
//	PORTD = 0;
//	PORTD = counter++;
	PORTD = sineWave[counter++];

//	counter++;
//	delay( 10 );
	delayMicroseconds( 5 );
}

#endif