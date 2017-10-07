
#include "SR04.h"

SR04::SR04(int echoPin, int triggerPin) {
    _echoPin = echoPin;
    _triggerPin = triggerPin;
    pinMode(_echoPin, INPUT);
    pinMode(_triggerPin, OUTPUT);
// pinMode( 8, OUTPUT );	// Power pin
    _autoMode = false;
    _distance = 999;
}


long SR04::Distance( int _waitDelay ) {
    long d = 0;
    _duration = 0;
    digitalWrite(_triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(_triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_triggerPin, LOW);
    delayMicroseconds(2);
    _duration = pulseIn(_echoPin, HIGH, PULSE_TIMEOUT);
    d = MicrosecondsToCentimeter(_duration);
    delay( _waitDelay );
    return d;
}

float SR04::Distance_Float( int _waitDelay ) {
    _duration = 0;

// digitalWrite( 8, HIGH );	// Power ON!
// delayMicroseconds( 2000000 );


    digitalWrite(_triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(_triggerPin, HIGH);
    delayMicroseconds(8);	// Patapom: after measuring with the oscilloscope we see that the Arduino adds some more µs, this is enough for an actual 10µs pulse
    digitalWrite(_triggerPin, LOW);
//    delayMicroseconds(2);
    _duration = pulseIn(_echoPin, HIGH, PULSE_TIMEOUT);
//_duration = pulseIn(_echoPin, HIGH, 6000);
    float	d = MicrosecondsToCentimeter_Float(_duration);
	if ( _waitDelay > 0 )
		delay( _waitDelay );

// digitalWrite( 8, LOW );	// Power OFF! (reset)

    return d;
}

long SR04::DistanceAvg(int wait, int count) {
    long min, max, avg, d;
    min = 999;
    max = 0;
    avg = d = 0;

    if (wait < 25) {
        wait = 25;
    }

    if (count < 1) {
        count = 1;
    }

    for (int x = 0; x < count + 2; x++) {
        d = Distance( wait );

        if (d < min) {
            min = d;
        }

        if (d > max) {
            max = d;
        }

        avg += d;
    }

    // substract highest and lowest value
    avg -= (max + min);
    // calculate average
    avg /= count;
    return avg;
}

float SR04::DistanceAvg_Float(int wait, int count) {
//     if ( wait < DEFAULT_DELAY ) {
//         wait = DEFAULT_DELAY;
//     }
    if (count < 1) {
        count = 1;
    }

	count += 2;	// Account for 2 more signals to at least get a min and a max distance
    float	min = 999.0f;
	float	max = 0.0f; 
	float	avg = 0.0f;
    for ( int x = 0; x < count; x++ ) {
        float	d = Distance_Float( wait );
        if ( d < min ) {
            min = d;
        } else if ( d > max ) {
            max = d;
        }

        avg += d;
    }

    // Subtract highest and lowest value
    avg -= max + min;

    // Calculate average
    avg /= count - 2;

    return avg;
}

void SR04::Ping() {
    _distance = Distance();
}

long SR04::getDistance() {
    return _distance;
}

long SR04::MicrosecondsToCentimeter(long duration) {
    long d = (duration * 100) / 5882;
    //d = (d == 0)?999:d;
    return d;
}

float SR04::MicrosecondsToCentimeter_Float(long duration) {
    return (duration * 100) / 5882.0f;
}




