// Lesson 20 - Sound Sensor
// Very disappointed: this sensor seems to be only a binary sensor reacting to a threshold
// When I attempted to use the analog read, I realized the sensitivity is very bad and it turns out
//	the device seems to only be calibrated to get very loud sounds, so this sensor is okay for
//	projects involving some "hand clapping" enabling or something like that, nothing more...
//
#define	PIN_ANALOG	A0
#define	PIN_DIGITAL	3
#define	PIN_LED		13
//#define	PIN_LED		9	// Use pin 9 for PWM output

void	setup()
{
	Serial.begin( 9600 );

	pinMode( PIN_LED, OUTPUT );
	pinMode( PIN_DIGITAL, INPUT );
	pinMode( PIN_ANALOG, INPUT );
}

int	sensorValue = 0;
int	counter = 0;
float	timer = 0;
void loop() {
	#if 1
		// Attempted to PWM display the collected wave amplitude but it turns out the sensitivity is very bad
		counter += 16;
		if ( counter > 1023 ) {
			counter = 0;
			sensorValue = analogRead( PIN_ANALOG );
//sensorValue = 511 + sin(timer+=0.0025f) * 511;
			Serial.println( sensorValue, DEC );
		}

// 		digitalWrite( PIN_LED, HIGH );
// 		digitalWrite( PIN_LED, LOW );
//		delay( 20 );
//		analogWrite( PIN_LED, 16);//sensorValue >> 4 );
		digitalWrite( PIN_LED, sensorValue > counter ? HIGH : LOW );
//		delay( 1 );

	#else
		// Binary mode when sound goes over a threshold (hand clap device?)
		int	sensorValue = digitalRead( PIN_DIGITAL );
		digitalWrite( PIN_LED, sensorValue );
		delay( 100 );
	#endif
}
