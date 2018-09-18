#include "Pom.h"

bool	bufferedRead( U8 _pin, int _expectedValue, U8 _duration, U8& _counter ) {
	int	value = digitalRead( _pin );
	if ( value == _expectedValue ) {
		_counter++;		// Increment counter of expected values
	} else {
		_counter = 0;	// Reset as soon as we have a different value than what we want
	}
	return _counter >= _duration;	// We only return true if we have got the expected values for at least the specified duration
}
