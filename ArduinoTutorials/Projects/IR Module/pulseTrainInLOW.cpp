unsigned short	pulseTrainInLOW__( volatile unsigned char* port, unsigned char _bit, unsigned long* _pulseTimes_us, unsigned short _maxPulses, unsigned long _endTrainTimeOutLoopsCount, unsigned long _timeOutLoopsCount ) {

	//////////////////////////////////////////////////////////////////////////
	// Store LOW and HIGH times in the user-specified array
	unsigned long*	pulseTime = _pulseTimes_us;
	unsigned short	pulseIndex = _maxPulses;
	unsigned long	maxloops = _timeOutLoopsCount;	// Use actual time out when waiting for first pulse to start

//	unsigned long long	startTime = micros();
	while ( pulseIndex != 0 ) {
		// Count high time
		while ( (*port & _bit) != 0 ) {
			if ( --maxloops == 0 ) {
				break;
			}
		}
		if ( maxloops == 0 ) {
			break;	// Train is over!
		}

		// Store high pulse-time
		*pulseTime++ = _endTrainTimeOutLoopsCount - maxloops;
		maxloops = _endTrainTimeOutLoopsCount;	// Reset counter to pulse train timeout
		pulseIndex--;
		if ( pulseIndex == 0 )
			break;	// Reached capacity

		// Count low time
		while ( (*port & _bit) == 0 ) {
			if ( --maxloops == 0 ) {
				break;
			}
		}
// 		if ( maxloops == 0 ) {
// 			break;	// Train is over!
// 		}

		// Store low pulse-time
		*pulseTime++ = _endTrainTimeOutLoopsCount - maxloops;
		maxloops = _endTrainTimeOutLoopsCount;	// Reset counter to pulse train timeout
		pulseIndex--;
	}

	return _maxPulses - pulseIndex;	// Train length
}
