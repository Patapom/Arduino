// This is LESSON 14 = IR Receiver Module
// ./Wiring.png
//
#include "Arduino.h"
#include "PulseTrain.h"

#define PIN	11

//static const byte	DELAY_US = 100;	// Sampling rate is 100µs (10KHz)
//static const int	TIME_START_SIGNAL_LOW_US = 9145;	// Start signal is a drop to LOW state for 9145µs
//static const int	TIME_START_SIGNAL_HIGH_US = 4540;	// ...followed by a rise to HIGH state for 4540µs
//
//static const byte	SAMPLES_COUNT_START_SIGNAL_LOW = TIME_START_SIGNAL_LOW_US / DELAY_US;
//static const byte	SAMPLES_COUNT_START_SIGNAL_HIGH = TIME_START_SIGNAL_HIGH_US / DELAY_US;

enum STATE {
	WAITING,
//	START_LOW,
//	START_HIGH,
	START,
	BIT_START,
	BIT_VALUE,
};

// bool	ExpectPulse( bool _state, int _duration ) {
// 	unsigned int	pulseDuration = pulseIn( PIN, _state ? HIGH : LOW, 2 * _duration );
// 	if ( pulseDuration < 9 * _duration / 10
// 		|| pulseDuration > 11 * _duration / 10 )
// 		return false;	// Outside the tolerance interval, failed!
// 
// 	return true;
// }

