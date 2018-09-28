//////////////////////////////////////////////////////////////////////////
// R-2R Resistor Ladder DAC Test, also a test for the TLC7528 DAC
// From Amanda Ghassaei's project (https://www.instructables.com/id/Arduino-Audio-Output/)
//////////////////////////////////////////////////////////////////////////
//
// Check noise reduction LC circuit from section 24.6.1 when plugging something into an analog input

#include "Pom/Pom.h"

// Pins to control the TLC7528 DAC
#define PIN_DACB	8			// !DACA/DACB
#define PIN_WR		9			// !WR

static const float	F = 32.0f;	// Channel restitution frequency (KHz)

U8	sineWaveL[256];
U8	sineWaveR[256];

#if 1

//////////////////////////////////////////////////////////////////////////
// User timer1 for generation
//
void setup2() {
	cli();	// Clear interrupt flag

	// Set data pins to output
	for ( int i=0; i < 8; i++ )
		pinMode( i, OUTPUT );

	pinMode( PIN_DACB, OUTPUT );
	pinMode( PIN_WR, OUTPUT );

//	float	f = 4*0.440f;		// Frequency (in KHz)
	float	f = 1.0f;		// Frequency (in KHz)

	#if 1
		float	t = 1.0f / f;				// Signal period (in ms)
		float	binTime = 1.0f / F;			// Duration of a single bin (in ms)
		float	binsCount = t / binTime;	// Amount of bins to cover the signal
		float	v = 2.0f * PI / binsCount;
	#else
		float	v = 2.0f * PI * f / F;
	#endif
	for ( int i=0; i < 256; i++ ) {
		sineWaveL[i] = U8( 127 + 127 * cosf( v * i ) );
		sineWaveR[i] = U8( 127 + 127 * cosf( v * i ) );
//sineWaveL[i] = (i & 1) ? 255 : 0;
	}

	Timer1::Init( Timer1::Clk1, Timer1::CTC_OCR1A );	// Clear Timer and Compare Match => Will interrupt when counter == OCR1A
	Timer1::SetOutputCompareA_CTCFrequency( 2.0f * F );	// Interrupt at 64KHz (32KHz for each channel, allows up to 16KHz reproductible frequency)
	Timer1::EnableInterrupts( true );

	sei();
}

static U8	counter = 0;
static bool	channelSelect = false;
ISR( TIMER1_COMPA_vect ) {
//	digitalWrite( PIN_WR, HIGH );	// Hold write
	PORTB |= 2;

	channelSelect = !channelSelect;
	if ( channelSelect ) {
		// Output left channel
//		digitalWrite( PIN_DACB, LOW );
		PORTB &= ~1;
		PORTD = sineWaveL[counter];
	} else {
		// Output right channel
//		digitalWrite( PIN_DACB, HIGH );
		PORTB |= 1;
		PORTD = sineWaveR[counter++];
	}

// channelSelect = !channelSelect;
// //PORTD = channelSelect ? 255 : 0;
// PORTD = sineWaveL[counter++];
// digitalWrite( PIN_DACB, LOW );

//	digitalWrite( PIN_WR, LOW );	// Resume write
	PORTB &= ~2;
}

void loop() {
#if 0
	// Update sinewave from serial port data
// 	static U8	serialCount = 0;
// 	Serial.readBytes( &sineWave[serialCount++], 1 );

//	if ( !Serial.available() )
//		return;

	Serial.readBytes( sineWaveL, 256 );
//	SerialPrintf( "Read %d bytes - 0x%x\n", readBytes, sineWave[0] );

#elif 0
	// Update the frequency
	static int	c = 0;
	static int	inc = 1;

	float	f = lerp( 0.1f, 2.0f, c / 127.0f );	// Interpolate between 100Hz and 2KHz
	float	v = 2.0f * PI * f / F;
 	for ( int i=0; i < 256; i++ ) {
 		sineWaveL[i] = U8( 127 + 127 * cosf( v * i ) );
 		sineWaveR[i] = U8( 127 + 127 * cosf( v * i ) );
 	}
// 	counter = 0;
// 	sineWaveL[counter] = U8( 127 + 127 * cosf( v * counter ) );
// 	sineWaveR[counter] = U8( 127 + 127 * cosf( v * counter ) );
// 	sineWaveL[counter+1] = U8( 127 + 127 * cosf( v * counter + v ) );
// 	sineWaveR[counter+1] = U8( 127 + 127 * cosf( v * counter + v ) );

	c += inc;
	if ( c == 0 )
		inc = 1;
	else if ( c > 127 )
		inc =-1;
#endif
}

#else
//////////////////////////////////////////////////////////////////////////
// Use main loop for generation
void setup2() {
	for ( int i=0; i < 8; i++ )
		pinMode( i, OUTPUT );
	for ( int i=0; i < 256; i++ )
		sineWaveL[i] = U8( 127 + 127 * sinf( 2.0f * PI * i / 255 ) );
}

void loop() {
	static U8	counter = 0;
//	PORTD = 0;
//	PORTD = counter++;
	PORTD = sineWaveL[counter++];

//	counter++;
//	delay( 10 );
	delayMicroseconds( 5 );
}

#endif