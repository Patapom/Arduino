#include "Arduino.h"
#include "PulseTrain.h"

// The core "pulseIn()" function (found in c:/program files/arduino/hardware/avr/cores/arduino/wiring_pulse.c) relies on the following function:
//	unsigned long pulseInSimpl(volatile uint8_t *port, uint8_t bit, uint8_t stateMask, unsigned long maxloops)
//	{
//	    unsigned long width = 0;
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
static const int	CYCLES_PER_LOOP = 16;	// Assume 16 cycles per inner loop iteration

#if 0

// ASM Version that should yield the same values whether we're in debug or release mode
// Except the timings are wrong, possibly because of additional cycles used for saving and
//	restoring context between the calls to micros()?
// I can't seem to figure out why we're having these issues and that makes the timings unusable... :(

extern "C" {
//	unsigned short	pulseTrainASM( volatile uint8_t* port, uint8_t _bit, uint32_t* _pulseTimes_us, uint16_t _maxPulses, uint32_t _endTrainTimeOutLoopsCount, uint32_t _timeOutLoopsCount );
	unsigned short	pulseTrainASM( volatile unsigned char* port, unsigned char _bit, unsigned long* _pulseTimes_us, unsigned short _maxPulses, unsigned long _endTrainTimeOutLoopsCount, unsigned long _timeOutLoopsCount );
}

uint32_t	pulseTrainInLOW( byte _pin, uint32_t* _pulseTimes_us, uint16_t _maxPulses, uint32_t _endTrainTimeOut_us, uint32_t _timeout_us ) {
	byte	bit = digitalPinToBitMask( _pin );
	byte	portIndex = digitalPinToPort( _pin );

	const uint32_t	timeOutLoopsCount = microsecondsToClockCycles( _timeout_us ) / CYCLES_PER_LOOP;
	const uint32_t	endTrainTimeOutLoopsCount = microsecondsToClockCycles( _endTrainTimeOut_us ) / CYCLES_PER_LOOP;

	//////////////////////////////////////////////////////////////////////////
	// Store LOW and HIGH times in the user-specified array
	uint64_t	startTime = micros();
	uint16_t	trainLength = pulseTrainASM( portInputRegister( portIndex ), bit, _pulseTimes_us, _maxPulses, endTrainTimeOutLoopsCount, timeOutLoopsCount );
	uint64_t	endTime = micros();
	if ( trainLength == 0 )
		return 0;	// Timed out!

// Serial.print( "Duration = " );
// Serial.println( (uint32_t) (endTime - startTime) );

//	endTime -= 50000;

	//////////////////////////////////////////////////////////////////////////
	// Transform LOW and HIGH times into actual µs

	// First, we need to compute an average factor that converts a loop iteration into a micro-second
	uint32_t	totalCount = 0;
	uint32_t*	pulseTime = _pulseTimes_us;
	*pulseTime += timeOutLoopsCount - endTrainTimeOutLoopsCount;	// Fix first count for proper timeout

	for ( uint16_t i=0; i < trainLength; i++ ) {
		uint32_t	pulseCount = *pulseTime++;
		totalCount += pulseCount;
//Serial.println( (unsigned long) pulseCount );
	}
	float	countToTime_us = float(endTime - startTime) / totalCount;	// Average counter to micro-second ratio

//countToTime_us *= 0.9f;

// Serial.print( "Total count = " );
// Serial.println( totalCount );

	// Next, we convert all the timings into micro-seconds
	// (and we shift all times by one since the first time is the high time count before the train actually started)
	trainLength--;
	pulseTime = _pulseTimes_us;
	for ( uint16_t i=0; i < trainLength; i++ ) {
		float	pulseTime_us = pulseTime[1] * countToTime_us;
		*pulseTime++ = uint32_t( pulseTime_us );
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
	//	unsigned short	pulseTrainASM( volatile uint8_t* port, uint8_t _bit, uint32_t* _pulseTimes_us, uint16_t _maxPulses, uint32_t _endTrainTimeOutLoopsCount, uint32_t _timeOutLoopsCount );
		unsigned short	pulseTrainASM( volatile unsigned char* port, unsigned char _bit, unsigned long* _pulseTimes_us, unsigned short _maxPulses, unsigned long _endTrainTimeOutLoopsCount, unsigned long _timeOutLoopsCount );
	}

#else
	// Simple C version
	unsigned short	pulseTrainASM( volatile unsigned char* port, unsigned char _bit, unsigned long* _pulseTimes_us, unsigned short _maxPulses, unsigned long _endTrainTimeOutLoopsCount, unsigned long _timeOutLoopsCount ) {
		uint32_t*	pulseTime = _pulseTimes_us;
		uint16_t	pulseIndex = _maxPulses;
		uint32_t	maxloops = _timeOutLoopsCount;	// Use actual time out when waiting for first pulse to start
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

uint32_t	pulseTrainInLOW( byte _pin, uint32_t* _pulseTimes_us, uint16_t _maxPulses, uint32_t _endTrainTimeOut_us, uint32_t _timeout_us ) {
	byte	bit = digitalPinToBitMask( _pin );
	byte	portIndex = digitalPinToPort( _pin );
	volatile uint8_t*	port = portInputRegister( portIndex );

	const uint32_t	timeOutLoopsCount = microsecondsToClockCycles( _timeout_us ) / CYCLES_PER_LOOP;
	const uint32_t	endTrainTimeOutLoopsCount = microsecondsToClockCycles( _endTrainTimeOut_us ) / CYCLES_PER_LOOP;

	//////////////////////////////////////////////////////////////////////////
	// Store LOW and HIGH times in the user-specified array
	uint32_t*	pulseTime = _pulseTimes_us;

	#if 1
		uint64_t	startTime = micros();
		uint16_t	trainLength = pulseTrainASM( port, bit, _pulseTimes_us, _maxPulses, endTrainTimeOutLoopsCount, timeOutLoopsCount );
		uint64_t	endTime = micros();
	#else
		uint16_t	pulseIndex = _maxPulses;
		uint32_t	maxloops = timeOutLoopsCount;	// Use actual time out when waiting for first pulse to start

		uint64_t	startTime = micros();
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
		uint64_t	endTime = micros();
		uint16_t	trainLength = _maxPulses - pulseIndex;
	#endif

	if ( trainLength == 0 )
		return 0;	// Timed out!

// Serial.print( "Duration = " );
// Serial.println( (uint32_t) (endTime - startTime) );

	//////////////////////////////////////////////////////////////////////////
	// Transform LOW and HIGH times into actual µs

// Serial.println( (unsigned long) timeOutLoopsCount );
// Serial.println( (unsigned long) endTrainTimeOutLoopsCount );

	// First, we need to compute an average factor that converts a loop iteration into a micro-second
	uint32_t	totalCount = 0;
	pulseTime = _pulseTimes_us;
// Serial.print( "Pulse time before = " );
// Serial.println( (unsigned long) *pulseTime );
	*pulseTime += timeOutLoopsCount - endTrainTimeOutLoopsCount;	// Fix first count for proper timeout
// Serial.print( "Pulse time after = " );
// Serial.println( (unsigned long) *pulseTime );
// Serial.println( "Pulse counts:" );

	for ( uint16_t i=0; i < trainLength; i++ ) {
		uint32_t	pulseCount = *pulseTime++;
		totalCount += pulseCount;
//Serial.println( (unsigned long) pulseCount );
	}
	float	countToTime_us = float(endTime - startTime) / totalCount;	// Average counter to micro-second ratio

// Serial.print( "Total count = " );
// Serial.println( (unsigned long) totalCount );
//Serial.println( countToTime_us );

	// Next, we convert all the timings into micro-seconds
	// (and we shift all times by one since the first time is the high time count before the train actually started)
	trainLength--;
	pulseTime = _pulseTimes_us;
	for ( uint16_t i=0; i < trainLength; i++ ) {
		float	pulseTime_us = pulseTime[1] * countToTime_us;
		*pulseTime++ = uint32_t( pulseTime_us );
// Serial.println( pulseTime_us );
	}

	return trainLength;
}
 
#else

uint32_t	pulseTrainInLOW( byte _pin, uint32_t* _pulseTimes_us, uint16_t _maxPulses, uint32_t _endTrainTimeOut_us, uint32_t _timeout_us ) {
	byte	bit = digitalPinToBitMask( _pin );
	byte	portIndex = digitalPinToPort( _pin );
	volatile uint8_t*	port = portInputRegister( portIndex );

	const uint32_t	timeOutLoopsCount = microsecondsToClockCycles( _timeout_us ) / CYCLES_PER_LOOP;
	const uint32_t	endTrainTimeOutLoopsCount = microsecondsToClockCycles( _endTrainTimeOut_us ) / CYCLES_PER_LOOP;

	//////////////////////////////////////////////////////////////////////////
	// Store LOW and HIGH times in the user-specified array
	uint32_t*	pulseTime = _pulseTimes_us;
	uint16_t	pulseIndex = _maxPulses;
	uint32_t	maxloops = timeOutLoopsCount;	// Use actual time out when waiting for first pulse to start

	uint64_t	startTime = micros();
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
	uint64_t	endTime = micros();
	uint16_t	trainLength = _maxPulses - pulseIndex;
	if ( trainLength == 0 )
		return 0;	// Timed out!

	//////////////////////////////////////////////////////////////////////////
	// Transform LOW and HIGH times into actual µs

// Serial.println( (unsigned long) timeOutLoopsCount );
// Serial.println( (unsigned long) endTrainTimeOutLoopsCount );

	// First, we need to compute an average factor that converts a loop iteration into a micro-second
	uint64_t	totalCount = 0;
	pulseTime = _pulseTimes_us;
// Serial.print( "Pulse time before = " );
// Serial.println( (unsigned long) *pulseTime );
	*pulseTime += timeOutLoopsCount - endTrainTimeOutLoopsCount;	// Fix first count for proper timeout
// Serial.print( "Pulse time after = " );
// Serial.println( (unsigned long) *pulseTime );

	for ( uint16_t i=0; i < trainLength; i++ ) {
		uint64_t	pulseCount = *pulseTime++;
		totalCount += pulseCount;
//Serial.println( (unsigned long) pulseCount );
	}
	float	countToTime_us = float(endTime - startTime) / totalCount;	// Average counter to micro-second ratio

// Serial.print( "Total count = " );
// Serial.println( (unsigned long) totalCount );
//Serial.println( countToTime_us );

	// Next, we convert all the timings into micro-seconds
	// (and we shift all times by one since the first time is the high time count before the train actually started)
	trainLength--;
	pulseTime = _pulseTimes_us;
	for ( uint16_t i=0; i < trainLength; i++ ) {
		float	pulseTime_us = pulseTime[1] * countToTime_us;
		*pulseTime++ = uint32_t( pulseTime_us );
//Serial.println( pulseTime_us );
	}

	return trainLength;
}

#endif