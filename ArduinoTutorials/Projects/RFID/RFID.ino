#define	PIN_DATA	12
#define	PIN_CS		11
#define	PIN_CLOCK	10

// The opcodes for the MAX7221 and MAX7219
#define OP_NOOP			0
#define OP_DIGIT0		1
#define OP_DIGIT1		2
#define OP_DIGIT2		3
#define OP_DIGIT3		4
#define OP_DIGIT4		5
#define OP_DIGIT5		6
#define OP_DIGIT6		7
#define OP_DIGIT7		8
#define OP_DECODEMODE	9
#define OP_INTENSITY	10
#define OP_SCANLIMIT	11
#define OP_SHUTDOWN		12
#define OP_DISPLAYTEST	15

//#define	DELAY_MS	10

void	SPITransfer( byte _opcode, byte _data ) {
	// Enable the line
	digitalWrite( PIN_CS, LOW );

	// Now shift out the opcode + data (a 16-bits value)
	shiftOut( PIN_DATA, PIN_CLOCK, MSBFIRST, _opcode );
	shiftOut( PIN_DATA, PIN_CLOCK, MSBFIRST, _data );

	// Latch the data onto the display
	digitalWrite( PIN_CS, HIGH );
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin( 9600 );
	pinMode( PIN_DATA, OUTPUT );
	pinMode( PIN_CLOCK, OUTPUT );
	pinMode( PIN_CS, OUTPUT );
	digitalWrite( PIN_CS, HIGH );

	SPITransfer( OP_SCANLIMIT, 7 );		// Scanlimit is set to max on startup
	SPITransfer( OP_DECODEMODE, 0 );	// No decode: we send raw bits!
	SPITransfer( OP_INTENSITY, 16 );	// Max intensity!
	SPITransfer( OP_DISPLAYTEST, 0 );	// Stop any current test
	SPITransfer( OP_SHUTDOWN, 1 );		// Wake up!
}

// the loop function runs over and over again until power down or reset
//byte	values[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
byte	values[8] = {	// Smiley face
	B00111100,
	B01000010,
	B10101001,
	B10101001,
	B10000101,
	B10111001,
	B01000010,
	B00111100,
};
void loop() {

	#if 1	// SINE WAVE
		// Clear values
		for ( byte row=0; row < 8; row++ )
			values[row] = 0;

		float	t = millis() * 0.001f;
		byte	value = 0x80;
		for ( byte column=0; column < 8; column++ ) {
			float	v = abs( sin( 2.0f * t ) );
			t += PI / 20.0f;
			byte	row = byte( min( 7, 8 * v ) );
			values[row] |= value;
			value >>= 1;
		}
	#endif

	// Send the values
	for ( byte row=0; row < 8; row++ )
		SPITransfer( OP_DIGIT0 + row, values[row] );

	#if 1	// Animate intensity
		float	intensity = 0.5f * (1.0f + sin( 0.001f * millis() ));
		SPITransfer( OP_INTENSITY, byte( min( 16, 16 * intensity*intensity ) ) );	// NOTE: using squared intensity for more linear look...
	#endif

	delay( 100 );
}
