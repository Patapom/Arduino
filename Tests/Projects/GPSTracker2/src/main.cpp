
// Wake →
//   Power GPS ON →
//   Attendre fix (max 15 s) →
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
#include "Global.h"

#include <Arduino.h>

#include <SPI.h>

// TFT screen (ST7789 v3.0)
#include <Adafruit_GFX.h>		// Core graphics library
#include <Adafruit_ST7789.h>	// Hardware-specific library for ST7789
#include "Modules/Display.h"

#define TFT_MOSI	23	// SDA Pin on ESP32
#define TFT_SCLK	18	// SCL Pin on ESP32
#define TFT_RST		33
#define TFT_DC		25	// Data Command control pin
#define TFT_CS		26	// Chip select control pin

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789	tft = Adafruit_ST7789( TFT_CS, TFT_DC, TFT_RST );
TFTDisplay		display( tft );

// GPS Module (Neo 6M)
#include "Modules/GPS.h"
GPS		gps( Serial1 );

#define	GPS_BAUD	9600
#define	PIN_GPS_RX	17
#define	PIN_GPS_TX	16

RawDegrees	homeLatitude { 49, 516223500, false };	// Home Latitude 49.516223500
RawDegrees	homeLongitude { 124, 362310167, true };	// Home Longitude -124.362310167

// LORA
#include "Modules/LORA.h"

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
	if ( !gps.FindFix( 10000 ) ) {
		display.println( "Initialization failed..." );
		display.println( "Couldn't find any satellite!" );
		while ( 1 );
	}

	delay( 1000 );

	display.println( "Awaiting GPS location data..." );
//*/

	startTime_ms = millis();
}

bool	foundFix = false;
bool	findingFix = false;

int		displayCounter = 0;
int		lastDisplayTime_ms = -1000;

void	ShowGPSDateTime();
void	ShowGPSData();

void	loop() {
//	Serial.println( "COUCOU!" );
//	delay( 1000 );
//	return;

/*	/////////////////////////////////////////////////////////////////////
	// Update display
	//
	int	now_ms = millis();
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
	// Update GPS
	//
	gps.ReadGPSData();

	ShowGPSData();

	// Send delta position through LORA
	RawDegrees	deltaLatitude;
	RawDegrees	deltaLongitude;

	GPS::Subtract( homeLatitude, gps.m_latitude, deltaLatitude );
	GPS::Subtract( homeLongitude, gps.m_longitude, deltaLongitude );

	lora.Sendf( 0,	// Broadcast to all!
				 "%d,%d",  deltaLatitude.negative  ? -S32( deltaLatitude.billionths )  : deltaLatitude.billionths
						, deltaLongitude.negative ? -S32( deltaLongitude.billionths ) : deltaLongitude.billionths
				);

	delay( 1000 );
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
		display.printf( "> Latitude %s%d.%09d\r\n", gps.m_latitude.negative ? "-" : "", gps.m_latitude.deg, gps.m_latitude.billionths );
		display.printf( "> Longitude %s%d.%09d\r\n", gps.m_longitude.negative ? "-" : "", gps.m_longitude.deg, gps.m_longitude.billionths );
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
