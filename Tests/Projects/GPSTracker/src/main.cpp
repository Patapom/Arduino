// Inspired by Robojax's code (https://robojax.com/learn/arduino/?vid=robojax_GPS_TinyGPSPlus)
//
// example received:
//	GPGSV,4,3,14,21,12,321,22,26,25,048,16,27,04,112,,29,00,003,*78
//	$GPGSV,4,4,14,30,08,244,,31,05,062,11*76
//	$GPGLL,4930.96847,N,12421.72920,W,220821.00,A,A*70
//	$GPRMC,220822.00,A,4930.96861,N,12421.72885,W,0.465,,300925,,,A*6A
//	$GPVTG,,T,,M,0.465,N,0.861,K,A*2B
//	$GPGGA,220822.00,4930.96861,N,12421.72885,W,1,09,0.96,72.8,M,-17.5,M,,*59
//	$GPGSA,A,3,16,31,03,09,26,21,04,07,11,,,,1.56,0.96,1.23*04
//	$GPGSV,4,1,14,03,22,171,18,04,63,102,20,06,14,264,,07,38,244,14*7C
//	$GPGSV,4,2,14,09,72,301,09,11,15,303,12,16,48,088,26,20,12,324,*71
//	$GPGSV,4,3,14,21,12,321,23,26,25,048,16,27,04,112,,29,00,003,*79
//	$GPGSV,4,4,14,30,08,244,,31,05,062,11*76
//	$GPGLL,4930.96861,N,12421.72885,W,220822.00,A,A*79
//	$GPRMC,220823.00,A,4930.96871,N,12421.72858,W,0.301,,300925,,,A*6F
//	$GPVTG,,T,,M,0.301,N,0.558,K,A*29
//	$GPGGA,220823.00,4930.96871,N,12421.72858,W,1,09,0.96,72.9,M,-17.5,M,,*58
//	$GPGSA,A,3,16,31,03,09,26,21,04,07,11,,,,1.56,0.96,1.23*04
//	$GPGSV,4,1,14,03,22,171,18,04,63,102,19,06,14,264,,07,38,244,14*76
//	$GPGSV,4,2,14,09,72,301,09,11,15,303,12,16,48,088,26,20,12,324,*71
//	$GPGSV,4,3,14,21,12,321,22,26,25,048,17,27,04,112,,29,00,003,*79
//	$GPGSV,4,4,14,30,08,244,,31,05,062,11*76
//	$GPGLL,4930.96871,N,12421.72858,W,220823.00,A,A*79
//	$GPRMC,220824.00,A,4930.96893,N,12421.72849,W,0.234,,300925,,,A*63
//	$GPVTG,,T,,M,0.234,N,0.433,K,A*22
//	$GPGGA,220824.00,4930.96893,N,12421.72849,W,1,09,0.96,73.0,M,-17.5,M,,*5B
//	$GPGSA,A,3,16,31,03,09,26,21,04,07,11,,,,1.56,0.96,1.23*04
//	$GPGSV,4,1,14,03,22,171,18,04,63,102,19,06,14,264,,07,38,244,14*76
//	$GPGSV,4,2,14,09,72,301,09,11,15,303,12,16,48,088,26,20,12,324,*71
//	$GPGSV,4,3,14,21,12,321,22,26,25,048,16,27,04,112,,29,00,003,*78
//	$GPGSV,4,4,14,30,08,244,,31,05,062,11*76
//	$GPGLL,4930.96893,N,12421.72849,W,220824.00,A,A*72
//
#include <Arduino.h>
//#include "SoftwareSerial.h"
#include <TinyGPSPlus.h>

// File system & SD card libraries
#include <SPI.h>
#include <SD.h>
#include <FS.h>

// GPS Communication & Decoding
static const int 		pinRX = D0, pinTX = D1;
static const uint32_t	GPSBaud = 9600;
//SoftwareSerial	SoftSerial( RXPin, TXPin ); // RX, TX

TinyGPSPlus		GPS;

// SD Card pins
//static const int 		pinMISO = D9, pinMOSI = D10, pinSCK = D8, pinCS = D2;
static const int 		pinMISO = GPIO_NUM_9, pinMOSI = GPIO_NUM_10, pinSCK = GPIO_NUM_8, pinCS = GPIO_NUM_4;

SPIClass	mySPI;

void	setup() {
	Serial.begin( 115200 );
	while ( !Serial.isConnected() ) {
		delay( 100 );
	}
essaie avec un vrai ESP32 et qu'on en finisse bordel !

	delay( 2000 );
	Serial.println( "Initializing..." );

	mySPI.begin( pinSCK, pinMISO, pinMOSI, pinCS );
	if ( !SD.begin( pinCS, mySPI, 4000000U ) ) {
		Serial.println( "Card mount Failed!" );
	}

	uint8_t	cardType = SD.cardType();
	if ( cardType == CARD_NONE ) {
		Serial.println( "No SD card attached!" );
//		return;
	} else {
		Serial.print( "SD Card Type: " );
		switch ( cardType ) {
			case CARD_MMC:
				Serial.println( "MMC" );
				break;
			case CARD_SD:
				Serial.println( "SDSC" );
				break;
			case CARD_SDHC:
				Serial.println( "SDHC" );
				break;
			default:
				Serial.println( "UNKNOWN" );
				break;
		}
	}

	uint64_t cardSize = SD.cardSize() / (1024 * 1024);
	Serial.printf( "SD Card Size: %lluMB\n", cardSize );

//return;

//	SoftSerial.begin( GPSBaud );
	Serial1.begin( GPSBaud, SERIAL_8N1, pinRX, pinTX );

	delay( 1000 );

	Serial.println( "Awaiting GPS location data..." );
}

void	loop() {
//	Serial.println( "COUCOU!" );
//	delay( 1000 );
//	return;

//if ( Serial1.available() ) {
//	Serial.print( (char) Serial1.read() );
//}
//return;

//	if ( SoftSerial.available() == 0 || !GPS.encode( SoftSerial.read() ) ) {
	if ( Serial1.available() == 0 || !GPS.encode( Serial1.read() ) ) {
		if ( millis() > 5000 && GPS.charsProcessed() < 10 ) {
			Serial.println( F("No GPS detected: check wiring.") );
			while(true);
		}
		delay( 100 );
		return;
	}

	if ( GPS.location.isValid() ) {
		Serial.print( "> Location " );
		Serial.print( GPS.location.lat(), 6 );
		Serial.print( ", " );
		Serial.print( GPS.location.lng(), 6 );
		Serial.println();
	} else {
		Serial.println( "Invalid GPS position..." );
	}
	delay( 1000 );
}
