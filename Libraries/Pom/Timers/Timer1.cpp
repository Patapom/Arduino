#include "../Pom.h"

void	Timer1::Init( 
		  ClockSelect _clock
		, WaveformGenerationMode _WGM
		, CompareOutputMode _compareModeA
		, CompareOutputMode _compareModeB
		, bool _enableNoiseCancelation
		, bool _inputCaptureOnRisingEdge
		, bool _forceOutputCompareChannelA
		, bool _forceOutputCompareChannelB
	) {

	// cf. section 16.11.1
	TCCR1A  = (_compareModeA << COM1A0)
			| (_compareModeB << COM1B0)
			| (_WGM & 0x3);

	// cf. section 16.11.2
	TCCR1B  = (_enableNoiseCancelation ? _BV(ICNC1) : 0)
			| (_inputCaptureOnRisingEdge ? _BV(ICES1) : 0)
			| ((_WGM & 0xC) << 1)
			| _clock;

	// cf. section 16.11.3
	TCCR1C  = (_forceOutputCompareChannelA ? _BV(FOC1A) : 0)
			| (_forceOutputCompareChannelB ? _BV(FOC1B) : 0);

// SerialPrintf( "TCCR1A = 0x%x\n", TCCR1A );
// SerialPrintf( "TCCR1B = 0x%x\n", TCCR1B );
// SerialPrintf( "TCCR1C = 0x%x\n", TCCR1C );
}

void	Timer1::EnableInterrupts(
	  bool _enableOutputCompareA
	, bool _enableOutputCompareB
	, bool _enableCounterOverflow
	, bool _enableInputCapture
) {
	TIMSK1  = (_enableOutputCompareA ? _BV(OCIE1A) : 0)
			| (_enableOutputCompareB ? _BV(OCIE1B) : 0)
			| (_enableCounterOverflow ? _BV(TOIE1) : 0)
			| (_enableInputCapture ? _BV(ICIE1) : 0);

//SerialPrintf( "TIMSK1 = 0x%x\n", TIMSK1 );
}

// Section 16.9.2 Clear Timer on Compare Match (CTC) Mode
// Gives the equation for the timer frequency as a function of the value set in OCR1A:
//
//		f = F_CPU / (2*N*(1+OCR1A))
//
// Where N is the pre-scaler divisor value
//
void	Timer1::SetOutputCompareA_CTCFrequency( float _frequency_KHz, ClockSelect _prescalerValue ) {
	float	N = 1.0f;
	switch ( _prescalerValue ) {
		case Clk8: N = 8.0f; break;
		case Clk64: N = 64.0f; break;
		case Clk256: N = 256.0f; break;
		case Clk1024: N = 1024.0f; break;
	}

	float	fValue = (F_CPU/2000.0f) / _frequency_KHz / N - 1;
	U16		value = U16( floorf( fValue ) );	// Round up
	SetOutputCompareA( value );

//SerialPrintf( "OCR1A Value = %d (%f KHz)\n", value, _frequency_KHz );
}
