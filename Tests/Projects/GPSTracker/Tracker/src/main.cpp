
// Wake →
//   Power GPS ON →
//   Attendre fix (max 15 s) →			<== ON N'A JAMAIS DE FIX EN SI PEU DE TEMPS !
//   Lire position →
//   Power GPS OFF →
//   Init LoRa →
//   Envoyer →
//   Sleep	(1 à 5 minutes selon qu'on a bougé ou non)
//
// Sleep:
//	esp_sleep_enable_timer_wakeup(60LL * 1000000); // 60 s
//	esp_deep_sleep_start();
//
// Optims:
// 	WiFi.mode(WIFI_OFF);
//	btStop();
//

// TODO:
//	• Gérer le GPS dans un thread à part et attendre qu'il donne de bonne valeurs
//		→ Pas besoin d'attendre un fix au setup...
//		→ Par contre, pas de sleep avant d'avoir un fix !
//
//#include <Global.h>

#include <Arduino.h>

#include <SPI.h>

// TFT screen (ST7789 v3.0)
#include <Adafruit_GFX.h>		// Core graphics library
#include <Adafruit_ST7789.h>	// Hardware-specific library for ST7789
#include <Display.h>

#define TFT_MOSI	23	// SDA Pin on ESP32
#define TFT_SCLK	18	// SCL Pin on ESP32
#define TFT_RST		33
#define TFT_DC		25	// Data Command control pin
#define TFT_CS		26	// Chip select control pin

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789	tft = Adafruit_ST7789( TFT_CS, TFT_DC, TFT_RST );
TFTDisplay		display( tft );

// GPS Module (Neo 6M)
#include <GPS.h>
GPS		gps( Serial1 );

#define	GPS_BAUD	9600
#define	PIN_GPS_RX	17
#define	PIN_GPS_TX	16

// LORA
#include <LORA.h>

#define	LORA_BAUD	115200
#define	PIN_LORA_RX	35
#define	PIN_LORA_TX	32

LORA	lora( 2 );


int		startTime_ms;
void	setup() {
	Serial.begin( 115200 );

	delay( 1000 );

	// =======================================================
	Serial.println( "Initializing SPI..." );

	SPI.begin( TFT_SCLK, -1, TFT_MOSI );


//*	// =======================================================
	Serial.println( "Initializing TFT Screen..." );

//Serial.printf( "TFT_RST = %d, TFT_DC = %d, TFT_CS = %d\r\n", TFT_RST, TFT_DC, TFT_CS );

//Serial.println("Before init");

	tft.init( 240, 280, SPI_MODE0 );	// Init ST7789 display 135x240 pixel
	tft.setRotation( 3 );

	display.Clear( 255, 128, 32 );
	display.SetTextProperties( 2, 0, 0, 0 );

//Serial.println("After init");
//*/

//*	// =======================================================
	display.println( "Initializing LORA module..." );

#if 0	// Functional!
	Serial2.begin( LORA_BAUD, SERIAL_8N1, PIN_LORA_RX, PIN_LORA_TX );
	delay( 1000 );

	Serial2.write( "AT+VER?\r\n" );

	// Read response
	char	buffer[256];
	bool	exit = false;
	while ( !exit ) {
		int	charsCount = Serial2.available();
		if ( charsCount == 0 )
			continue;

		Serial2.read( buffer, charsCount );
		for ( int i=0; i < charsCount; i++ ) {
			char	C = buffer[i];
			Serial.print( C );
			if ( C == '\n' )
				exit = true;
		}
	}
#endif

	if ( !lora.Begin( Serial2, LORA_BAUD, PIN_LORA_RX, PIN_LORA_TX ) ) {
		display.println( "Initialization failed..." );
		display.printf( "Last error → %s\r\n", lora.LastErrorString() );
		display.printf( "Return → %s\r\n", lora.m_receiveBuffer );
		while ( 1 );
	}
//*/

//*	// =======================================================
	display.println( "Initializing GPS..." );

	Serial1.begin( GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX );

//	delay( 1000 );

	// Provide guess for initial position
	gps.UBXSendInitialPosition( homeLatitude, homeLongitude, homeAltitude_m );

	// Try to find a fix for 15 seconds
//	GPS::FIX_STATUS	fixStatus = gps.FindFix( 15000 );
	GPS::FIX_STATUS	fixStatus = gps.FindFix( -1 );	// No time out
	if ( fixStatus != GPS::FIX_STATUS::FOUND_FIX ) {
		display.println( "Initialization failed..." );
		switch ( fixStatus ) {
			case GPS::FIX_STATUS::ERROR_TIME_OUT: display.println( "Couldn't find any satellite!" ); break;
			case GPS::FIX_STATUS::ERROR_NO_GPS_MODULE: display.println( "GPS module not found after 5s. Check wiring!" ); break;
			default: display.println( "Couldn't find any satellite!" ); break;
		}
		while ( 1 );
	}

	display.println( "Awaiting GPS location data..." );
//*/


	// =======================================================
	display.println( "Initializing buzzer..." );

	ledcAttachPin( 27, 0 );
	ledcSetup( 0, 2000, 8 );	// 2 kHz
//	ledcWrite( 0, 128 );		// Bzzz!
	ledcWrite( 0, 0 );

	startTime_ms = millis();
}

bool	foundFix = false;
bool	findingFix = false;

int		displayCounter = 0;
int		lastDisplayTime_ms = -1000;
int		lastBroadcastTime_ms = -1000;

bool	buzzerEnabled = false;

void	ShowGPSDateTime();
void	ShowGPSData();
void	BroadcastGPSData();
bool	ProcessCommand( U16 _transmitterID, const char* _message, U8 _messageLength );

void	loop() {
	int	now_ms = millis();

//	Serial.println( "COUCOU!" );
//	delay( 1000 );
//	return;

	if ( buzzerEnabled ) {
		// Funky localization sound
//		static int 	counter = 0;
//		ledcWrite( 0, counter++ & 0xFF );		// Bzzz!

		#if 1	// FUNKY! :D 
			U8	sine = U8( 128 + 127 * sin( 2*PI * (now_ms - startTime_ms) / 1000.0f ) );
sine >>= 5;
			ledcWrite( 0, sine );		// Bzzz!
		#else	// Change frequency
Actually poses a problem as it reconfigures the timer every time...
//			U32	frequency = U32( 1500.0f + 500.0f * sin( 2*PI * (now_ms - startTime_ms) / 1000.0f ) );	// Oscillates between 500 Hz and 2000 Hz
//			ledcChangeFrequency( 0, frequency, 8 );
//			ledcWrite( 0, 128 );
		#endif
	} else {
		ledcWrite( 0, 0 );
	}

/*	/////////////////////////////////////////////////////////////////////
	// Update display
	//
	if ( now_ms - lastDisplayTime_ms > 1000 ) {
		displayCounter++;

		if ( displayCounter & 1 ) {
			display.PrintTest();
		} else {
			display.PrintTest2();
		}
		tft.invertDisplay( displayCounter & 2 );

		lastDisplayTime_ms = now_ms;
	}
//*/

	////////////////////////////////////////////////////////////////////
	// Read LORA
	//
	U16		transmitterID;
	U8		payloadLength;
	S16		RSSI, SNR;
	bool	error;
	const char*	message = lora.Receive( transmitterID, payloadLength, RSSI, SNR, error );
	if ( message != nullptr ) {
		ProcessCommand( transmitterID, message, payloadLength );
	} else if ( error ) {
		Serial.printf( "Error reading message: %s\r\n", lora.LastErrorString() );
	}


	////////////////////////////////////////////////////////////////////
	// Update GPS
	//
	gps.ReadGPSData();

//	ShowGPSData();

//	// Broadcast position every second
//	if ( now_ms - lastBroadcastTime_ms > 1000 ) {
//		lastBroadcastTime_ms = now_ms;
//		BroadcastGPSData();
//	}
}

// Send delta position through LORA (we're only sending the billionth of degrees relative to home, usually 3 to 4 characters long for short distances)
void	BroadcastGPSData() {

	#if 1	// Use simple double precision floats

	double	deltaLatitude = gps.m_avgLatitude - homeLatitude;
	double	deltaLongitude = gps.m_avgLongitude - homeLongitude;

//Serial.printf( "Delta lat = %f / %f\r\n", deltaLatitude, deltaLongitude );

	int		intDeltaLatitude = 10000000 * deltaLatitude;
	int		intDeltaLongitude = 10000000 * deltaLongitude;

//Serial.printf( "Integer Delta lat/long = %d / %d\r\n", intDeltaLatitude, intDeltaLongitude );
display.printf( "Dlat/lng = %d / %d\r\n", intDeltaLatitude, intDeltaLongitude );

	lora.Sendf( 0,	// Broadcast to all!
				"G%d,%d", intDeltaLatitude, intDeltaLongitude	// Prefix with G to indicate GPS data
				);

	#else	// Use raw degrees operations
	RawDegrees	deltaLatitude;
	RawDegrees	deltaLongitude;

// We're not using the actual home location as moving to the south-east will always yield very large billionth values
	GPS::Subtract( gps.m_latitude, homeLatitude, deltaLatitude );
	GPS::Subtract( gps.m_longitude, homeLongitude, deltaLongitude );
//// Instead we subtract the minimum corner of the island so we always get positive values
//	GPS::Subtract( gps.m_latitude, minLatitude, deltaLatitude );
//	GPS::Subtract( gps.m_longitude, minLongitude, deltaLongitude );

Serial.printf( "Delta lat = %d.%09d ", deltaLatitude.negative ? -deltaLatitude.deg : deltaLatitude.deg, deltaLatitude.billionths );
Serial.printf( "Delta lon = %d.%09d\r\n", deltaLongitude.negative ? -deltaLongitude.deg : deltaLongitude.deg, deltaLongitude.billionths );

	lora.Sendf( 0,	// Broadcast to all!
				 "%d,%d",  deltaLatitude.negative  ? -S32( deltaLatitude.billionths )  : deltaLatitude.billionths
						, deltaLongitude.negative ? -S32( deltaLongitude.billionths ) : deltaLongitude.billionths
				);
#endif
}

bool	ProcessCommand( U16 _transmitterID, const char* _message, U8 _messageLength ) {
Serial.printf( "Processing command \"%s\"\r\n", _message );

	// Process the "BUZZ=<ON/OFF>" command
	if ( strstr( _message, "CBUZZ=" ) == _message ) {
		_message += 6;
		_messageLength -= 6;
		buzzerEnabled = _messageLength == 2 && _message[0] == 'O' && _message[1] == 'N';

		display.println( buzzerEnabled ? "Buzzer enabled" : "Buzzer disabled" );

		// Send ACK
		lora.Send( _transmitterID, buzzerEnabled ? "ACKBUZZ1" : "ACKBUZZ0" );
		return true;
	}

	return false;
}


void	ShowGPSData() {

	ShowGPSDateTime();

//	if ( !GPS.location.isValid() ) {
//		if ( findingFix ) {
//			display.print( "." );
//		} else {
//			findingFix = true;
//			if ( !foundFix ) {
//				display.print( "Invalid GPS position → Waiting for a fix" );
//			} else {
//				display.print( "Lost fix → Waiting for a fix" );
//			}
//		}
//
//		return;
//	}
//
//	foundFix = true;
//	if ( findingFix ) {
//		display.println( " FOUND!" );
//		findingFix = false;
//	}

	if ( gps.m_locationQuality != GPS::Invalid ) {
//		display.printf( "> Location %f, %f\r\n", gps.lat(), gps.lng() );
		display.printf( "> Latitude %f\r\n", gps.m_latitude );
		display.printf( "> Longitude %f\r\n", gps.m_longitude );
	}
}

void	ShowGPSDateTime() {
	if ( !gps.isDateTimeValid() )
		return;

//	// UTC
//	display.printf( "> UTC Date %04d/%02d/%02d\r\n", gps.m_dateTime.Y, gps.m_dateTime.M, gps.m_dateTime.D );
//	display.printf( "> UTC Time %02d:%02d:%02d\r\n", gps.m_dateTime.h, gps.m_dateTime.m, gps.m_dateTime.s );

	// Local
	GPS::DateTime	localDateTime;
	gps.m_dateTime.ToLocal( localDateTime );

	display.printf( "> Date %04d/%02d/%02d\r\n", localDateTime.Y, localDateTime.M, localDateTime.D );
	display.printf( "> Time %02d:%02d:%02d\r\n", localDateTime.h, localDateTime.m, localDateTime.s );
}
