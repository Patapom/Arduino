#include "../Pom.h"

// The core "pulseIn()" function (found in c:/program files/arduino/hardware/avr/cores/arduino/wiring_pulse.c) relies on the following function:
//	U32 pulseInSimpl(volatile U8 *port, U8 bit, U8 stateMask, U32 maxloops)
//	{
//	    U32 width = 0;
//	    // wait for any previous pulse to end
//	    while ((*port & bit) == stateMask)
//	        if (--maxloops == 0)
//	            return 0;
//
//	    // wait for the pulse to start
//	    while ((*port & bit) != stateMask)
//	        if (--maxloops == 0)
//	            return 0;
//
//	    // wait for the pulse to stop
//	    while ((*port & bit) == stateMask) {
//	        if (++width == maxloops)
//	            return 0;
//	    }
//	    return width;
//	}
//
// This function was "optimized" into an ASM routine found in c:/program files/arduino/hardware/avr/cores/arduino/wiring_pulse.s
//
// We will simply copy its core mechanism (without the ASM conversion at the moment) but for multiple pulses.
//
static const U32	CYCLES_PER_LOOP = 16;	// Assume 16 cycles per inner loop iteration

#if 0

// ASM Version that should yield the same values whether we're in debug or release mode
// Except the timings are wrong, possibly because of additional cycles used for saving and
//	restoring context between the calls to micros()?
// I can't seem to figure out why we're having these issues and that makes the timings unusable... :(

extern "C" {
//	U16	pulseTrainASM( volatile U8* port, U8 _bit, U32* _pulseTimes_us, U16 _maxPulses, U32 _endTrainTimeOutLoopsCount, U32 _timeOutLoopsCount );
	U16	pulseTrainASM( volatile U8* port, U8 _bit, U32* _pulseTimes_us, U16 _maxPulses, U32 _endTrainTimeOutLoopsCount, U32 _timeOutLoopsCount );
}

U32	pulseTrainInLOW( U8 _pin, U32* _pulseTimes_us, U16 _maxPulses, U32 _endTrainTimeOut_us, U32 _timeout_us ) {
	U8	bit = digitalPinToBitMask( _pin );
	U8	portIndex = digitalPinToPort( _pin );

	const U32	timeOutLoopsCount = microsecondsToClockCycles( _timeout_us ) / CYCLES_PER_LOOP;
	const U32	endTrainTimeOutLoopsCount = microsecondsToClockCycles( _endTrainTimeOut_us ) / CYCLES_PER_LOOP;

	//////////////////////////////////////////////////////////////////////////
	// Store LOW and HIGH times in the user-specified array
	U32	startTime = micros();
	U16	trainLength = pulseTrainASM( portInputRegister( portIndex ), bit, _pulseTimes_us, _maxPulses, endTrainTimeOutLoopsCount, timeOutLoopsCount );
	U32	endTime = micros();
	if ( trainLength == 0 )
		return 0;	// Timed out!

// Serial.print( "Duration = " );
// Serial.println( (U32) (endTime - startTime) );

//	endTime -= 50000;

	//////////////////////////////////////////////////////////////////////////
	// Transform LOW and HIGH times into actual µs

	// First, we need to compute an average factor that converts a loop iteration into a micro-second
	U32	totalCount = 0;
	U32*	pulseTime = _pulseTimes_us;
	*pulseTime += timeOutLoopsCount - endTrainTimeOutLoopsCount;	// Fix first count for proper timeout

	for ( U16 i=0; i < trainLength; i++ ) {
		U32	pulseCount = *pulseTime++;
		totalCount += pulseCount;
//Serial.println( (U32) pulseCount );
	}
	float	countToTime_us = float(endTime - startTime) / totalCount;	// Average counter to micro-second ratio

//countToTime_us *= 0.9f;

// Serial.print( "Total count = " );
// Serial.println( totalCount );

	// Next, we convert all the timings into micro-seconds
	// (and we shift all times by one since the first time is the high time count before the train actually started)
	trainLength--;
	pulseTime = _pulseTimes_us;
	for ( U16 i=0; i < trainLength; i++ ) {
		float	pulseTime_us = pulseTime[1] * countToTime_us;
		*pulseTime++ = U32( pulseTime_us );
//Serial.println( pulseTime_us );
	}

	return trainLength;
}

#elif 1

#if 0
	// ASM Version that should yield the same values whether we're in debug or release mode
	// Except the timings are wrong, possibly because of additional cycles used for saving and
	//	restoring context between the calls to micros()?
	// I can't seem to figure out why we're having these issues and that makes the timings unusable... :(

	extern "C" {
	//	U16	pulseTrainASM( volatile U8* port, U8 _bit, U32* _pulseTimes_us, U16 _maxPulses, U32 _endTrainTimeOutLoopsCount, U32 _timeOutLoopsCount );
		U16	pulseTrainASM( volatile U8* port, U8 _bit, U32* _pulseTimes_us, U16 _maxPulses, U32 _endTrainTimeOutLoopsCount, U32 _timeOutLoopsCount );
	}

#else
	// Simple C version
	U16	pulseTrainASM( volatile U8* port, U8 _bit, U32* _pulseTimes_us, U16 _maxPulses, U32 _endTrainTimeOutLoopsCount, U32 _timeOutLoopsCount ) {
		U32*	pulseTime = _pulseTimes_us;
		U16		pulseIndex = _maxPulses;
		U32		maxloops = _timeOutLoopsCount;	// Use actual time out when waiting for first pulse to start
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

		return _maxPulses - pulseIndex;
	}
#endif

U32	pulseTrainInLOW( U8 _pin, U32* _pulseTimes_us, U16 _maxPulses, U32 _endTrainTimeOut_us, U32 _timeout_us ) {
	U8	bit = digitalPinToBitMask( _pin );
	U8	portIndex = digitalPinToPort( _pin );
	volatile U8*	port = portInputRegister( portIndex );

	const U32	timeOutLoopsCount = microsecondsToClockCycles( _timeout_us ) / CYCLES_PER_LOOP;
	const U32	endTrainTimeOutLoopsCount = microsecondsToClockCycles( _endTrainTimeOut_us ) / CYCLES_PER_LOOP;

	//////////////////////////////////////////////////////////////////////////
	// Store LOW and HIGH times in the user-specified array
	U32*	pulseTime = _pulseTimes_us;

	#if 1
		U32	startTime = micros();
		U16	trainLength = pulseTrainASM( port, bit, _pulseTimes_us, _maxPulses, endTrainTimeOutLoopsCount, timeOutLoopsCount );
		U32	endTime = micros();
	#else
		U16	pulseIndex = _maxPulses;
		U32	maxloops = timeOutLoopsCount;	// Use actual time out when waiting for first pulse to start

		U32	startTime = micros();
		while ( pulseIndex != 0 ) {
			// Count high time
			while ( (*port & bit) != 0 ) {
				if ( --maxloops == 0 ) {
					break;
				}
			}
			if ( maxloops == 0 ) {
				break;	// Train is over!
			}

			// Store high pulse-time
			*pulseTime++ = endTrainTimeOutLoopsCount - maxloops;
			maxloops = endTrainTimeOutLoopsCount;	// Reset counter to pulse train timeout
			pulseIndex--;
			if ( pulseIndex == 0 )
				break;	// Reached capacity

			// Count low time
			while ( (*port & bit) == 0 ) {
				if ( --maxloops == 0 ) {
					break;
				}
			}
// 			if ( maxloops == 0 ) {
// 				break;	// Train is over!
// 			}

			// Store low pulse-time
			*pulseTime++ = endTrainTimeOutLoopsCount - maxloops;
			maxloops = endTrainTimeOutLoopsCount;	// Reset counter to pulse train timeout
			pulseIndex--;
		}
		U32	endTime = micros();
		U16	trainLength = _maxPulses - pulseIndex;
	#endif

	if ( trainLength == 0 )
		return 0;	// Timed out!

// Serial.print( "Duration = " );
// Serial.println( (U32) (endTime - startTime) );

	//////////////////////////////////////////////////////////////////////////
	// Transform LOW and HIGH times into actual µs

// Serial.println( (U32) timeOutLoopsCount );
// Serial.println( (U32) endTrainTimeOutLoopsCount );

	// First, we need to compute an average factor that converts a loop iteration into a micro-second
	U32	totalCount = 0;
	pulseTime = _pulseTimes_us;
// Serial.print( "Pulse time before = " );
// Serial.println( (U32) *pulseTime );
	*pulseTime += timeOutLoopsCount - endTrainTimeOutLoopsCount;	// Fix first count for proper timeout
// Serial.print( "Pulse time after = " );
// Serial.println( (U32) *pulseTime );
// Serial.println( "Pulse counts:" );

	for ( U16 i=0; i < trainLength; i++ ) {
		U32	pulseCount = *pulseTime++;
		totalCount += pulseCount;
//Serial.println( (U32) pulseCount );
	}
	float	countToTime_us = float(endTime - startTime) / totalCount;	// Average counter to micro-second ratio

// Serial.print( "Total count = " );
// Serial.println( (U32) totalCount );
//Serial.println( countToTime_us );

	// Next, we convert all the timings into micro-seconds
	// (and we shift all times by one since the first time is the high time count before the train actually started)
	trainLength--;
	pulseTime = _pulseTimes_us;
	for ( U16 i=0; i < trainLength; i++ ) {
		float	pulseTime_us = pulseTime[1] * countToTime_us;
		*pulseTime++ = U32( pulseTime_us );
// Serial.println( pulseTime_us );
	}

	return trainLength;
}
 
#else

U32	pulseTrainInLOW( U8 _pin, U32* _pulseTimes_us, U16 _maxPulses, U32 _endTrainTimeOut_us, U32 _timeout_us ) {
	U8	bit = digitalPinToBitMask( _pin );
	U8	portIndex = digitalPinToPort( _pin );
	volatile U8*	port = portInputRegister( portIndex );

	const U32	timeOutLoopsCount = microsecondsToClockCycles( _timeout_us ) / CYCLES_PER_LOOP;
	const U32	endTrainTimeOutLoopsCount = microsecondsToClockCycles( _endTrainTimeOut_us ) / CYCLES_PER_LOOP;

	//////////////////////////////////////////////////////////////////////////
	// Store LOW and HIGH times in the user-specified array
	U32*	pulseTime = _pulseTimes_us;
	U16	pulseIndex = _maxPulses;
	U32	maxloops = timeOutLoopsCount;	// Use actual time out when waiting for first pulse to start

	U32	startTime = micros();
	while ( pulseIndex != 0 ) {
		// Count high time
		while ( (*port & bit) != 0 ) {
			if ( --maxloops == 0 ) {
				break;
			}
		}
		if ( maxloops == 0 ) {
			break;	// Train is over!
		}

		// Store high pulse-time
		*pulseTime++ = endTrainTimeOutLoopsCount - maxloops;
		maxloops = endTrainTimeOutLoopsCount;	// Reset counter to pulse train timeout
		pulseIndex--;
		if ( pulseIndex == 0 )
			break;	// Reached capacity

		// Count low time
		while ( (*port & bit) == 0 ) {
			if ( --maxloops == 0 ) {
				break;
			}
		}
// 		if ( maxloops == 0 ) {
// 			break;	// Train is over!
// 		}

		// Store low pulse-time
		*pulseTime++ = endTrainTimeOutLoopsCount - maxloops;
		maxloops = endTrainTimeOutLoopsCount;	// Reset counter to pulse train timeout
		pulseIndex--;
	}
	U32	endTime = micros();
	U16	trainLength = _maxPulses - pulseIndex;
	if ( trainLength == 0 )
		return 0;	// Timed out!

	//////////////////////////////////////////////////////////////////////////
	// Transform LOW and HIGH times into actual µs

// Serial.println( (U32) timeOutLoopsCount );
// Serial.println( (U32) endTrainTimeOutLoopsCount );

	// First, we need to compute an average factor that converts a loop iteration into a micro-second
	U32	totalCount = 0;
	pulseTime = _pulseTimes_us;
// Serial.print( "Pulse time before = " );
// Serial.println( (U32) *pulseTime );
	*pulseTime += timeOutLoopsCount - endTrainTimeOutLoopsCount;	// Fix first count for proper timeout
// Serial.print( "Pulse time after = " );
// Serial.println( (U32) *pulseTime );

	for ( U16 i=0; i < trainLength; i++ ) {
		U32	pulseCount = *pulseTime++;
		totalCount += pulseCount;
//Serial.println( (U32) pulseCount );
	}
	float	countToTime_us = float(endTime - startTime) / totalCount;	// Average counter to micro-second ratio

// Serial.print( "Total count = " );
// Serial.println( (U32) totalCount );
//Serial.println( countToTime_us );

	// Next, we convert all the timings into micro-seconds
	// (and we shift all times by one since the first time is the high time count before the train actually started)
	trainLength--;
	pulseTime = _pulseTimes_us;
	for ( U16 i=0; i < trainLength; i++ ) {
		float	pulseTime_us = pulseTime[1] * countToTime_us;
		*pulseTime++ = U32( pulseTime_us );
//Serial.println( pulseTime_us );
	}

	return trainLength;
}

#endif