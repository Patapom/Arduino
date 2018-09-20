extern void	setup2();
void setup() {
	Serial.begin( 19200 );
	while ( !Serial.availableForWrite() );

	setup2();
}
