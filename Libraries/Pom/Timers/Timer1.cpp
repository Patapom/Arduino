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
	TCCR1B  = (_forceOutputCompareChannelA ? _BV(FOC1A) : 0)
			| (_forceOutputCompareChannelB ? _BV(FOC1B) : 0);
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
}
