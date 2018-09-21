// Check noise reduction LC circuit from section 24.6.1 when plugging something into an analog input

#include "Pom/Pom.h"

void setup2() {
//	cli();

// 1) Set ClockSelect => internal
// 2) Je pense qu'il faut passer en waveform pour monter/descendre et diviser la fréquence par 2 et avoir 16KHz
// 	=> Non, il faut mettre le TOP à 65536 >> 8 pour que ça boucle 256 fois plus vite et qu'on ait une PWM de 31372.55 * 256 Hz
// 	=> A chaque interrupt, on balance une nouvelle donnée?
// 3) set "wave form generation" TCCR1A et TCCR1B
// 
// 
// An alternative will then be to use the fast PWM mode using OCR1A for
// defining TOP (WGM13:0 = 15) since the OCR1A then will be double buffered.
// 
// 
// For generating a waveform output in CTC mode, the OC1A output can be set to toggle its logical level on each
// compare match by setting the Compare Output mode bits to toggle mode (COM1A1:0 = 1). The OC1A value will
// not be visible on the port pin unless the data direction for the pin is set to output (DDR_OC1A = 1). The
// waveform generated will have a maximum frequency of fOC1A = fclk_I/O/2 when OCR1A is set to zero (0x0000).
// The waveform frequency is defined by the following equation:
// 
// 	FOC1A = Fclk / (2*N*(1+OCR1A))	N = prescaler = 1
// 
// 16.9.3 Fast PWM Mode
// This high frequency makes the fast PWM mode well suited for power regulation, rectification, and DAC applications.
// 
// 
// 
// 
// loop => Ecrire OCR1A aussi vite que possible (i.e. 15KHz) pour piloter l'amplitude du signal
//  => Non, utiliser l'interrupt pour ça!


// 	Timer1::Init(	Timer1::ClockSelect::Clk1,						// Setup PWM frequency divisor to 1 for 31372.55 Hz frequency
// 					Timer1::WaveformGenerationMode::FastPWM_8Bits	// Set TOP to 256 so we have 8-bits resolution in our PWM, while operating at 32KHz
// 				);
// 
// 	Timer1::EnableInterrupts( false, false, true );	// Enable timer 1 overflow interrupt

	pinMode( 2, OUTPUT );
	pinMode( 3, OUTPUT );

	pinMode( 8, OUTPUT );
	pinMode( 9, OUTPUT );

//	sei();
}

//////////////////////////////////////////////////////////////////////////
// Timer1 is set to FastPWN mode with 8-bits precision
//
// volatile U32	timerCounter = 0;
// volatile float	counterFactor = 0.01f * 2.0f * PI;	// 3KHz
// ISR( TIMER1_OVF_vect ) {
// 	float	amplitude = 0.5f * (1.0f + sinf( timerCounter * counterFactor ));
// 	Timer1::SetOutputCompareA( U8(amplitude * 255.0f) );	// PWM is set to 8-bits precision
// 	timerCounter++;
// }
// 
// void	SetSoundFrequency( float _frequencyHz ) {
// 	// Basically, the timer counter is 
// 	// Will reach "timerFrequency" each second
// 	const float		timerFrequency = 31372.55f;
// 	float	counterFactor = 2.0f * PI / timerFrequency;
// volatile float	soundFrequency = 1000.0f;	// 1KHz
// }


//////////////////////////////////////////////////////////////////////////

void loop() {
#if 1

	U32		shift = 5;
	U32		count = 100 << shift;
	U32		inc = 1;
	U64		nextMicros = micros() + (count >> shift);

	while ( true ) {
		if ( micros() < nextMicros )
//		if ( micros() - nextMicros < count )
			continue;

		nextMicros = nextMicros + (count >> shift);
		PIND = 0b00000100;	 // output on PortD-2 (is D3?)

// Change frequency
count += inc;
if ( count > 800<<shift )		inc = -1;
else if ( count < 50<<shift )	inc = +1;
	}

#else
//	Serial.println( "LOOP" );
	static bool	state = false;
	delay( 200 );
	digitalWrite( 8, state );
//	digitalWrite( 9, !state );
	state != true;
#endif
}