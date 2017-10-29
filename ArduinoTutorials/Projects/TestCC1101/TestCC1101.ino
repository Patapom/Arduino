#include "Pom/Pom.h"
#include "CC1101/CC1101.h"

#define	PIN_GDO0	8
#define	PIN_GDO2	9
#define	PIN_CS		10
#define	PIN_SI		11
#define	PIN_SO		12	// Also GDO1
#define	PIN_CLOCK	13

// the setup function runs once when you press reset or power the board
Pom::CC1101	C( PIN_CS, PIN_CLOCK, PIN_SI, PIN_SO, PIN_GDO0, PIN_GDO2 );


// http://www.st.com/content/ccc/resource/technical/document/application_note/2f/bb/7f/94/76/fa/4b/3c/DM00054821.pdf/files/DM00054821.pdf/jcr:content/translations/en.DM00054821.pdf
// EN 300 220 
//  868.3 MHz center frequency.
// The data rate is set to 9.6 kbps.
// The frequency deviation is set to 2.4 kHz, and the modulation is set to gaussian FSK (GFSK) with a BT = 1. 
//
void setup() {
	Serial.begin( 9600 );

//C.Reset();
C.SetNormalTransferMode();

/*	// Read back frequencies
	Serial.print( "Freq = " );
	Serial.print( C.GetCarrierFrequency() );
	Serial.println( " MHz" );
	Serial.print( "Intermediate Freq = " );
	Serial.print( C.GetIntermediateFrequency() );
	Serial.println( " KHz" );
	float	a, b;
	C.GetChannelBandwithAndDataRate( a, b );
	Serial.print( "Bandwidth = " );
	Serial.print( a );
	Serial.println( " KHz" );
	Serial.print( "Data rate = " );
	Serial.print( b );
	Serial.println( " KBd" );
	Serial.print( "Spacing = " );
	Serial.print( C.GetChannelSpacing() );
	Serial.println( " KHz" );
	Serial.print( "Deviation = " );
	Serial.print( C.GetFrequencyDeviation() );
	Serial.println( " KHz" );
//*/
/*	// Test functions
	Serial.println( "Testing Functions" );
	C.SetAddress();
	C.EnableWhitening();
	C.SetPacketFormat();
	C.EnableCRC();
	C.SetPacketLengthConfig();
	C.EnablePacketAddressCheck();
	C.SetPacketLength();
	C.SetSyncWord();

	C.SetChannel();

	C.SetCarrierFrequency();
	C.SetIntermediateFrequency();
	C.SetFrequencyOffset();
	C.SetChannelBandwithAndDataRate();
	C.SetChannelSpacing();
	C.SetFrequencyDeviation();

//	C.SetGDOx( Pom::CC1101::GDO0 );
//	C.SetGDOx( Pom::CC1101::GDO2 );

	Serial.println( "DONE!" );

	// Read state
	Serial.print( "Machine state = " );
	Serial.println( C.ReadFSMState(), HEX );
//*/
/*	// Write registers
	Serial.println( "Dumping register values..." );
	
	byte	initialValues[0x3D];
	C.DumpAllRegisters( initialValues );

	for ( byte i=0; i < 0x3D; i++ ) {
		Serial.print( "0x" );
	 	Serial.print( i, HEX );
		Serial.print( " = 0x" );
	 	Serial.print( initialValues[i], HEX );
		Serial.println();
	}
//*/
}

// the loop function runs over and over again until power down or reset
void loop() {

	#if 0
		// Send a bisou!
		const char*	string = "BISOU!";
		C.Transmit( 6, string );
	
//Serial.print( digitalRead( PIN_GDO2 ) ? "1 " : "0 " );

		// Wait a little
//		delay( 500 );
	#else
		// Receive a bisou!
		byte	buffer[256];
		byte	size = C.Receive( buffer );
		if ( size > 0 ) {
			buffer[size] = '\0';	// Add trailing 0 to end string
			Serial.print( "Received " );
			Serial.print( size );
			Serial.print( " bytes \"" );
			Serial.print( (const char*) buffer );
			Serial.println( "\"" );
		}
	#endif
}
