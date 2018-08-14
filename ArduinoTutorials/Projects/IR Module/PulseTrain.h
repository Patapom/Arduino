// This is a helper that is modeled on the core "pulseIn()" function but that rather waits for a train of pulses instead of a single pulse.
//

// Measures a collection of pulse lengths in a train of LOW state pulses
//	_pin, the index of the pin to read
//	_pulseTimes_us, a pointer to an array of train times (in micro seconds) for each pulse.
//					The array will contain a series of LOW, HIGH, LOW, HIGH, times from the first low pulse to end high pulse
//	_maxPulses, the maximum amount of pulse times that can be stored in the array
//	_endTrainTimeOut_us, the time after which a high pulse is esteemed to represent the end of the pulse train (e.g. twice the longest possible high pulse in the train)
//	_timeout_us, (default = 1s) the time after which we should return with a timeout (use 0 for infinite wait; i.e. until a pulse)
//
uint32_t	pulseTrainInLOW( byte _pin, uint32_t* _pulseTimes_us, uint16_t _maxPulses, uint32_t _endTrainTimeOut_us, uint32_t _timeout_us=1000000 );

