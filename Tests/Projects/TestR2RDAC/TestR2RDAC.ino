extern void	setup2();
void setup() {
//	Serial.begin( 19200 );
	Serial.begin( 115200 );
	while ( !Serial.availableForWrite() );

	setup2();
}
