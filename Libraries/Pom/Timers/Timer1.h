// Timer1 helper class
// Description of timer 1 functionalities can be found in Section 16 of the ATMEL datasheet
//
class Timer1 {

public:

	enum ClockSelect {
		None = 0,					// No clock source (Timer/Counter stopped)
		Clk1,						// clk / 1 (No prescaling)
		Clk8,						// clk / 8 (From prescaler)
		Clk64,						// clk / 64 (From prescaler)
		Clk256,						// clk / 256 (From prescaler)
		Clk1024,					// clk / 1024 (From prescaler)
		External_FallingEdge,		// External clock source on T1 pin. Clock on falling edge.
		External_RisingEdge,		// External clock source on T1 pin. Clock on rising edge.
	};

	enum WaveformGenerationMode {
		Normal = 0,					// Normal. TOP = 0XFFFF
		PWM_8Bits,					// PWM, Phase Correct, 8-bit.  TOP = 0xFF
		PWM_9Bits,					// PWM, Phase Correct, 9-bit.  TOP = 0x1FF
		PWM_10Bits,					// PWM, Phase Correct, 10-bit. TOP = 0x3FF
		CTC_OCR1A,					// Clear Timer and Compare match (CTC). TOP = OCR1A
		FastPWM_8Bits,				// Fast PWM, 8-bit.  TOP = 0xFF
		FastPWM_9Bits,				// Fast PWM, 9-bit.  TOP = 0x1FF
		FastPWM_10Bits,				// Fast PWM, 10-bit. TOP = 0x3FF
		PWM_PhaseFreqCorrect_ICR1,	// PWM, Phase and Frequency Correct. TOP = ICR1
		PWM_PhaseFreqCorrect_OCR1A,	// PWM, Phase and Frequency Correct. TOP = OCR1A
		PWM_PhaseCorrect_ICR1,		// PWM, Phase Correct. TOP = ICR1
		PWM_PhaseCorrect_OCR1A,		// PWM, Phase Correct. TOP = OCR1A
		CTC_ICR1,					// Clear Timer and Compare match (CTC). TOP = ICR1
		// (Reserved)
		FastPWM_ICR1 = 14,			// Fast PWM. TOP = ICR1
		FastPWM_OCR1A,				// Fast PWM. TOP = OCR1A
	};

	enum CompareOutputMode {
		Normal_,					// Normal port operation, output compare pin OC1 is disabled
		ToggleOnCompareMatch,		// Toggle OC1 on match
		ClearOnCompareMatch,		// Clear OC1 on match
		SetOnCompareMatch,			// Set OC1 on match
	};

public:

	static void	Init(
		  ClockSelect _clock
		, WaveformGenerationMode _WGM
		, CompareOutputMode _compareModeA = CompareOutputMode::Normal_
		, CompareOutputMode _compareModeB = CompareOutputMode::Normal_
		, bool _enableNoiseCancelation=true
		, bool _inputCaptureOnRisingEdge=true
		, bool _forceOutputCompareChannelA=false
		, bool _forceOutputCompareChannelB=false
	);

	// Call this to enable interrupts
	// Example:
	//	{
	//		cli();	// Clear interrupt flag
	//		Timer1::EnableInterrupts( true );	// Enable interrupts when timer reaches value set in OCR1A
	//		sei();	// Set interrupt flag
	//	}
	//
	//	ISR( TIMER1_COMPA_vect ) {
	//		(...) code for handling the interrupt
	//	}
	//
	static void	EnableInterrupts(
		  bool _enableOutputCompareA = false
		, bool _enableOutputCompareB = false
		, bool _enableCounterOverflow = false
		, bool _enableInputCapture = false
	);

	static U16	GetCounter()					{ return TCNT1; }
	static void	SetCounter( U16 _value )		{ TCNT1 = _value; }
	static U16	GetOutputCompareA()				{ return OCR1A; }
	static void	SetOutputCompareA( U16 _value )	{ OCR1A = _value; }
	static U16	GetOutputCompareB()				{ return OCR1B; }
	static void	SetOutputCompareB( U16 _value )	{ OCR1B = _value; }
	static U16	GetInputCapture()				{ return ICR1; }
	static void	SetInputCapture( U16 _value )	{ ICR1 = _value; }
};
