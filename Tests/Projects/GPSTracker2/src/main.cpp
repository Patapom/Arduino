
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
#include <TinyGPSPlus.h>

#define	GPS_BAUD	9600
#define	PIN_GPS_RX	17
#define	PIN_GPS_TX	16
TinyGPSPlus		GPS;

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

#if 0 // Basic serial printing of GPS data
	if ( Serial1.available() ) {
		Serial.print( (char) Serial1.read() );
	}
	return;
#endif

	if ( Serial1.available() == 0 || !GPS.encode( Serial1.read() ) ) {
		if ( (millis() - startTime_ms) > 5000 && GPS.charsProcessed() < 10 ) {
			display.println( "No GPS detected: check wiring." );
			while(true);
		}
		delay( 100 );
		return;
	}

	ShowGPSData();
	delay( 1000 );
}

void	ShowGPSData() {

	ShowGPSDateTime();

	if ( !GPS.location.isValid() ) {
		if ( findingFix ) {
			display.print( "." );
		} else {
			findingFix = true;
			if ( !foundFix ) {
				display.print( "Invalid GPS position → Waiting for a fix" );
			} else {
				display.print( "Lost fix → Waiting for a fix" );
			}
		}

		return;
	}

	foundFix = true;
	if ( findingFix ) {
		display.println( " FOUND!" );
		findingFix = false;
	}

	if ( GPS.location.isValid() ) {
		display.printf( "> Location %f, %f\r\n", GPS.location.lat(), GPS.location.lng() );
	}
}

time_t gpsToUnixUTC( struct tm& t ) {
	static const int days_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};

	int year = t.tm_year + 1900;
	int month = t.tm_mon + 1;

	// années depuis 1970
	long days = 0;
	for (int y = 1970; y < year; y++) {
		days += ( (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0) ) ? 366 : 365;
	}

	for (int m = 1; m < month; m++) {
		days += days_month[m-1];
		if (m == 2 && ((year%4==0 && year%100!=0) || (year%400==0)))
			days += 1;
	}

	days += (t.tm_mday - 1);

	return days * 86400
			+ t.tm_hour * 3600
			+ t.tm_min * 60
			+ t.tm_sec;
}

// So apparently we can get a "valid" time and date that is clearly wrong, even without a location fix...
// I think it's best to wait for a proper satellite fix before reading the date & time! (it can take a while though :/)
//
void	ShowGPSDateTime() {
	if ( !GPS.date.isValid() || !GPS.time.isValid() )
		return;
	
	struct tm	utc;
				utc.tm_year = GPS.date.year() - 1900;
				utc.tm_mon  = GPS.date.month() - 1;
				utc.tm_mday = GPS.date.day();
				utc.tm_hour = GPS.time.hour();
				utc.tm_min  = GPS.time.minute();
				utc.tm_sec  = GPS.time.second();

	// UTC time zone
	setenv( "TZ", "UTC0", 1 );
	tzset();

	// Convert into timestamp
//	time_t		t = timegm( &t );	// Non standard... Doesn't exist on ESP32
//	time_t		t = mktime( &utc );
	time_t		t = gpsToUnixUTC( utc );

	// Vancouver Time Zone
	setenv( "TZ", "PST8PDT,M3.2.0,M11.1.0", 1 );
	tzset();

	struct tm*	local = localtime( &t );
				local->tm_year += 1900;
				local->tm_mon++;

	// UTC
	display.printf( "> UTC Date %04d/%02d/%02d\r\n", GPS.date.year(), GPS.date.month(), GPS.date.day() );
	display.printf( "> UTC Time %02d:%02d:%02d\r\n", GPS.time.hour(), GPS.time.minute(), GPS.time.second() );

	// Local
	display.printf( "> Date %04d/%02d/%02d\r\n", local->tm_year, local->tm_mon, local->tm_mday );
	display.printf( "> Time %02d:%02d:%02d\r\n", local->tm_hour, local->tm_min, local->tm_sec );
}
