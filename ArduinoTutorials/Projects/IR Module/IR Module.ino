#include "IRModule.h"

void setup() {
	Serial.begin( 9600 );
	pinMode( PIN, INPUT );
	pinMode( LED_BUILTIN, OUTPUT );
}

bool	ExpectTime( uint32_t _pulseTime, uint32_t _expectedTime, int _tolerancePercentage ) {
	uint32_t	minTime = (_expectedTime * (100 - _tolerancePercentage)) / 100;
	uint32_t	maxTime = (_expectedTime * (100 + _tolerancePercentage)) / 100;
	return minTime < _pulseTime && _pulseTime < maxTime;
}

void loop() {

	// As measured from the oscilloscope captures:
	//	• First drop to LOW state is 9.145ms long
	//	• Followed by a 4.54ms delay in HIGH state
	//	• Then data bits are sent as a drop to LOW state for 594µs and...
	//		• Either a rise to HIGH state for 594µs (bit = 0)
	//		• Or a rise to HIGH state for 1671µs (bit = 1)
	//
	uint32_t	pulseTimes[128];
	uint16_t	pulsesCount = pulseTrainInLOW( PIN, pulseTimes, 128, 18000 );

	if ( !pulsesCount ) {
		return;
	}

	if ( pulsesCount == 3 ) {
	 	Serial.println( "KEEP PRESSING" );
		return;
	}

	if ( pulsesCount != 67 )
		return;
	if ( !ExpectTime( pulseTimes[0], 9000, 20 ) || !ExpectTime( pulseTimes[1], 4500, 20 ) ) {
	 	Serial.println( "INVALID PULSE TRAIN HEADER!" );
		return;
	}

#if 0
	Serial.println();
	Serial.print( "Pulses Count = " );
	Serial.println( pulsesCount );
	for ( uint16_t i=0; i < pulsesCount; i++ ) {
		Serial.println( pulseTimes[i] );
	}
#else
	Serial.println( "Message: " );

	uint8_t	values[4];
	int	pulseIndex = 2;
	for ( int i=0; i < 4; i++ ) {
		uint8_t	value = 0;
		uint8_t	bitMask = 0x80U;
		for ( int bit=0; bit < 8; bit++, bitMask>>=1 ) {
			if ( !ExpectTime( pulseTimes[pulseIndex++], 600, 20 ) ) {
	 			Serial.println( "INVALID PULSE LOW TIME! (expected 600µs)" );
	 			Serial.println( pulseTimes[pulseIndex-1] );
				return;
			}
			uint32_t	time = pulseTimes[pulseIndex++];
			bool		isZero = ExpectTime( time, 600, 20 );
			bool		isOne = ExpectTime( time, 1600, 20 );
			if ( !isZero && !isOne ) {
	 			Serial.println( "INVALID PULSE HIGH TIME! (expected 600µs or 1600µs)" );
	 			Serial.println( time );
				return;
			}
			value |= isOne ? bitMask : 0;
		}
		values[i] = value;
	 	Serial.println( value, HEX );
	}
#endif
}
