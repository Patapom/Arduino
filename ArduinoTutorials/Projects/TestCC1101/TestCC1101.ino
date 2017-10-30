#include "Pom/Pom.h"
#include "CC1101/CC1101.h"
//#include "ELECHOUSE_CC1101.h"

#define	RECEIVE	2	// Define this to receive, comment to transmit

#define	PIN_GDO0	2
#define	PIN_GDO2	9
#define	PIN_CS		10
#define	PIN_SI		11
#define	PIN_SO		12	// Also GDO1
#define	PIN_CLOCK	13

// the setup function runs once when you press reset or power the board
Pom::CC1101	C( PIN_CS, PIN_CLOCK, PIN_SI, PIN_SO, PIN_GDO0, PIN_GDO2 );

//ELECHOUSE_CC1101	C2;

// http://www.st.com/content/ccc/resource/technical/document/application_note/2f/bb/7f/94/76/fa/4b/3c/DM00054821.pdf/files/DM00054821.pdf/jcr:content/translations/en.DM00054821.pdf
// EN 300 220 
// 868.3 MHz center frequency.
// The data rate is set to 9.6 kbps.
// The frequency deviation is set to 2.4 kHz, and the modulation is set to gaussian FSK (GFSK) with a BT = 1. 
//

void setup() {
	Serial.begin( 19200 );
	while ( !Serial.availableForWrite() );

	pinMode( 3, OUTPUT );

	#if RECEIVE
		if ( !C.EnterReceiveMode() )
			Serial.println( "Failed at entering RX mode! (time out)" );
	#endif
	#if RECEIVE == 2
		// Enable asynchronous mode: plug-in the oscilloscope probe onto GDO2 pin to observe received data in real time
		C.SetAsynchronousTransferMode();
	#else
		C.SetNormalTransferMode();
	#endif

//	C2.Init();
//	#if RECEIVE
//		C2.SetReceive();
//	#endif

#if 0
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
#endif
}

bool	state = true;
void loop() {
//	Serial.println( "LOOP" );

//	#if RECEIVE
//		if ( C2.CheckReceiveFlag() ) {
//			byte	buffer[256];
//			byte	size = C2.ReceiveData( buffer );
//			Serial.print( "Received " );
//			Serial.print( size );
//			Serial.print( " bytes \"" );
//			Serial.print( (const char*) buffer );
//			Serial.println( "\"" );
//
//			C2.SetReceive();
//		} else {
//			delay( 100 );
//		}
//	#else
//		const char*	string = "BISOU!";
//		C2.SendData( string, 6 );
//
//		delay( 500 );
//	#endif

//*
	#if RECEIVE == 1
		if ( digitalRead( PIN_GDO0 ) ) {
			while ( digitalRead( PIN_GDO0 ) );

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

				delay( 500 );
			}
			Serial.println( "AMOUR!" );
		} else {

			delay( 50 );
//			//	FREQEST			= 0x32, // Frequency Offset Estimate
//			//	LQI				= 0x33, // Demodulator estimate for Link Quality
//			//	RSSI			= 0x34, // Received signal strength indication
//			//	MARCSTATE		= 0x35, // Control state machine state
//			//	WORTIME1		= 0x36, // High U8 of WOR timer
//			//	WORTIME0		= 0x37, // Low U8 of WOR timer
//			//	PKTSTATUS		= 0x38, // Current GDOx status and packet status
//			//	VCO_VC_DAC		= 0x39, // Current setting from PLL calibration module
//			//	TXBYTES			= 0x3A, // Underflow and number of bytes in the TX FIFO
//			//	RXBYTES			= 0x3B, // Overflow and number of bytes in the RX FIFO
//			//	RCCTRL1_STATUS	= 0x3C, // Last RC oscillator calibration result
//			//	RCCTRL0_STATUS	= 0x3D, // Last RC oscillator calibration result
//			C.DumpManyStates( 0x35, 0 );
//			C.SendCommandStrobe( 0x3A );

		}
	#elif RECEIVE == 2
		// Asynchronous spying mode
		// TODO: Log in a bunch of input bits, analyze...
	#else
		// Send a bisou!
		const char*	string = "BISOU!";
		U8	transmittedSize = C.Transmit( 6, (U8*) string );
		if ( transmittedSize != 6 ) {
			Serial.println( "Failed to transmit entire data!" );
			Serial.println( transmittedSize );
//		} else {
//			Serial.println( "SUCCESS!" );
		}
	
//C.DisplayStatusRegisters();
//Serial.print( digitalRead( PIN_GDO2 ) ? "1 " : "0 " );

		// Wait a little
		delay( 500 );
	#endif
//*/

	digitalWrite( 3, state );
	state = !state;
}
