// Lesson 22 + A piece of lesson 33 using the rotary encoder
extern void setup2();

void setup() {
	Serial.begin(9600);
	while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
//	Serial.println( F("BORDEL!") );
	setup2();
}
