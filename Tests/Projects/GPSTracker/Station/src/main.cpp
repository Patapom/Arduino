
//
#include <Arduino.h>

// I2C & SPI
#include <Wire.h>
#include <SPI.h>

// File system & SD card libraries
#include <SD.h>
#include <FS.h>

// TFT screen (ST7789 v3.0)
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

// Magnetometer (BNO 055)
#include <BNO055.h>


#if 1	// ESP32
static const int 		pinMISO = GPIO_NUM_19, pinMOSI = GPIO_NUM_23, pinSCK = GPIO_NUM_18, pinCS = GPIO_NUM_5;	// SD Card pins
static const int		pinSDA = GPIO_NUM_21, pinSCL = GPIO_NUM_22;	// I2C for accelerometer
#else	// XIAO
//static const int 		pinMISO = D9, pinMOSI = D10, pinSCK = D8, pinCS = D2;
static const int 		pinMISO = GPIO_NUM_9, pinMOSI = GPIO_NUM_10, pinSCK = GPIO_NUM_8, pinCS = GPIO_NUM_4;	// SD Card pins
static const int 		pinRX = GPIO_NUM_0, pinTX = GPIO_NUM_1;		// GPS Communication & Decoding
static const int		pinSDA = , pinSCL = ;
#endif

// LORA
#include <LORA.h>	// RYLR998 Lora Communication

#define	LORA_BAUD	115200
#define	PIN_LORA_RX	35
#define	PIN_LORA_TX	32

LORA	lora( 1 );	// STATION ID


bool	FindFixProgress( const GPS::Data& _data, U32 _elapsedTime_ms, void* _parameter ) {
	Serial.print( "." );
	return true;	// Continue...
}

void	ShowGPSData();

int	startTime_ms;

void	setup() {
	Serial.begin( 115200 );
//	while ( !Serial.isConnected() ) {
//		delay( 100 );
//	}

	delay( 1000 );

	pinMode( pinCS, OUTPUT );	// SD CS
	pinMode( TFT_CS, OUTPUT );	// TFT CS

	digitalWrite( pinCS, HIGH );	// désactive SD
	digitalWrite( TFT_CS, HIGH );	// désactive TFT

	// =======================================================
	Serial.println( "Initializing SPI & I2C..." );

//	mySPI.begin( pinSCK, pinMISO, pinMOSI, pinCS );
	SPI.begin( pinSCK, pinMISO, pinMOSI );

	Wire.begin( pinSDA, pinSCL );
//	Wire.begin( pinSDA, pinSCL, 100000 );


//*	// =======================================================
	Serial.println( "Initializing TFT Screen..." );

	tft.init( 240, 280, SPI_MODE0 );	// Init ST7789 display 135x240 pixel
	tft.setRotation( 3 );

	display.Clear( 0xFF, 0xF0, 0x10 );
	display.SetTextProperties( 2, 0, 0, 0 );

	digitalWrite( TFT_CS, HIGH );
//*/

/*	// =======================================================
	Serial.println( "Initializing SD card module..." );

	digitalWrite( pinCS, LOW );

//	if ( !SD.begin( pinCS, mySPI ) ) {
	if ( !SD.begin( pinCS ) ) {
//	if ( !SD.begin( pinCS, mySPI, 4000000U ) ) {
		Serial.println( "Card mount Failed!" );
		return;
	}

	uint8_t	cardType = SD.cardType();
	if ( cardType == CARD_NONE ) {
		Serial.println( "No SD card attached!" );
		return;
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

	digitalWrite( pinCS, HIGH );
//*/

/*	// =======================================================
	Serial.println( "Initializing magnetometer module..." );

//Wire.beginTransmission(0x28);
//byte error = Wire.endTransmission();
//Serial.println(error); // 0 = OK

	ScanI2C();

//delay(1000);

	if ( !bno.begin() ) {
		Serial.println( "BNO055 not detected!" );
		while ( 1 );
	}

	delay( 1000 );

	bno.setExtCrystalUse( true );	// Use external reference

	// Show calibration values
	GetCalibration();	// MAG should be at 3 to be working properly!

//	if ( ShouldCalibrate() ) {
//		Serial.println( "Entering calibration mode..." );
//		Calibrate();
//	}
//*/


//*	// =======================================================
	display.println( "Initializing LORA module..." );

//	Serial2.begin( LORABaud, SERIAL_8N1, pinRX2, pinTX2 );
//	delay( 1000 );
//
//	Serial2.write( "AT+VER?\r\n" );
//
//	// Read response
//	char	buffer[256];
//	bool	exit = false;
//	while ( !exit ) {
//		int	charsCount = Serial2.available();
//		if ( charsCount == 0 )
//			continue;
//
//		Serial2.read( buffer, charsCount );
//		for ( int i=0; i < charsCount; i++ ) {
//			char	C = buffer[i];
//			Serial.print( C );
//			if ( C == '\n' )
//				exit = true;
//		}
//	}

	if ( !lora.Begin( Serial2, LORA_BAUD, PIN_LORA_RX, PIN_LORA_TX ) ) {
		display.println( "Initialization failed..." );
		display.printf( "Error!\r\n-> %s\r\n", lora.LastErrorString() );
		while ( 1 );
	}

//*/

//*	// =======================================================
	display.println( "Initializing GPS..." );

//	Serial1.begin( GPSBaud, SERIAL_8N1, pinRX, pinTX );

	Serial1.begin( GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX );

//	delay( 1000 );
//	// Empty buffer
	while ( Serial1.available() )
		Serial1.read();
	Serial1.flush();

	// Provide guess for initial position
	gps.UBXSendInitialPosition( homeLatitude, homeLongitude, homeAltitude_m );

	// Wait for ACK
//gps.UBXSend( 0x0A, 0x04, nullptr, 0 );	// UBX-MON-VER => Not sure what it's supposed to return?
//
//// Apparently never comes: command ignored or just no ACK on this command?
//while (  true ) {
//	if ( Serial1.available() == 0 )
//		continue;
//
//	int	v = Serial1.read();
//	Serial.print( v, HEX );
//	Serial.print( " (" );
//	Serial.print( (char ) v );
//	Serial.print( ") " );
//}
//	if ( gps.UBXWaitForAck( 0x13, 0x40, 10000 ) )
//		Serial.println( "ACK!" );
//	else
//		Serial.println( "NOT ACK!" );


	#if 1	// Async task
		gps.StartMonitoringTask();
	#else	// Sync task
		// Try to find a fix for 15 seconds
//		GPS::FIX_STATUS	fixStatus = gps.FindFix( FindFixProgress, 15000 );
		GPS::FIX_STATUS	fixStatus = gps.FindFix( FindFixProgress, nullptr, -1 );	// No time out
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
	#endif
//*/

	startTime_ms = millis();
}

int		lastCommandTime_ms = 0;
bool	enableRemoteBuzzer = false;

void	ProcessMessage( const char* _payload, U8 _payloadLength );
void	DisplayGPSStatus( const GPS::Data& _data );

void	loop() {
	int	now_ms = millis();

//	Serial.println( "COUCOU!" );
//	delay( 1000 );
//	return;

//	if ( bno.isFullyCalibrated() )
//		ShowMagnetometerData();

	////////////////////////////////////////////////////////////////////
	// Read GPS status
	GPS::Data	GPSData;
	if ( gps.GetGPSData( GPSData ) ) {
		DisplayGPSStatus( GPSData );
	}

/*	////////////////////////////////////////////////////////////////////
	// Send commands to the tracker module
	if ( now_ms - lastCommandTime_ms > 3500 ) {
		lastCommandTime_ms = now_ms;

		enableRemoteBuzzer = !enableRemoteBuzzer;
		lora.Send( 2, enableRemoteBuzzer ? "CBUZZ=ON" : "CBUZZ=OFF" );

		display.println( enableRemoteBuzzer ? "Remote buzzer ON" : "Remote buzzer OFF" );
	}
//*/


//*	////////////////////////////////////////////////////////////////////
	// Wait for LORA messages
	U16			transmitterID;
	U8			payloadLength;
	S16			RSSI, SNR;
	bool		error;
	const char*	payload = lora.Receive( transmitterID, payloadLength, RSSI, SNR, error );
	if ( payload == nullptr ) {
		if ( error ) {
			Serial.printf( "Error reading message: %s\r\n", lora.LastErrorString() );
		}
		return;
	}

	if ( payloadLength == 0 ) {
		return;
	} else if ( payload[0] != 'G' ) {
		// Process something else...
		ProcessMessage( payload, payloadLength );
		return;
	}

	// Process a GPS latitude/longitude
Serial.printf( "Received \"%s\" (%d, %d, %d)\r\n", payload, transmitterID, RSSI, SNR );

	char*	strDeltaLatitude = (char*) payload;
	char*	strDeltaLongitude = strstr( strDeltaLatitude, "," );
	if ( strDeltaLongitude == nullptr ) {
		display.print( "Invalid lat/lon message!" );
		return;
	}
	*strDeltaLongitude++ = '\0';

	int	deltaLatitude = atoi( strDeltaLatitude );
	int	deltaLongitude = atoi( strDeltaLongitude );

	double	targetLatitude = homeLatitude + deltaLatitude / 1000000000.0;
	double	targetLongitude = homeLongitude + deltaLongitude / 1000000000.0;

//Serial.printf( "Lat/Lon = %f / %f\r\n", targetLatitude, targetLongitude );

	// Compute direction & distance
//	double	currentLatitude = homeLatitude;
//	double	currentLongitude = homeLongitude;
	double	currentLatitude = gps.m_data.avgLatitude;
	double	currentLongitude = gps.m_data.avgLongitude;

	float	distance_m;
	float	bearing = GPS::ComputeDirection( currentLatitude, currentLongitude, targetLatitude, targetLongitude, distance_m );
	display.printf( "Dir %3.1f' @ %.1fm\r\n", bearing, distance_m );
//*/
}

void	ProcessMessage( const char* _payload, U8 _payloadLength ) {
	if ( strstr( _payload, "ACK" ) == _payload ) {
		// Process command acknowledge
Serial.printf( "Recognized ACK for command %s\r\n", _payload+3 );
	} else {
Serial.printf( "Unknown payload \"%s\"\r\n", _payload );
	}
}

bool	findingFix = false;
bool	foundFix = false;
void	DisplayGPSStatus( const GPS::Data& _data ) {
	delay( 1000 );

	switch ( _data.fixStatus ) {
		case GPS::FIX_STATUS::ERROR_NO_GPS_MODULE:
			display.printf( "No GPS module detected! Task aborted...\r\n" );
			break;

		case GPS::FIX_STATUS::NO_FIX:
			if ( !findingFix ) {
				if ( foundFix )
					display.printf( "Lost fix! Searching\r\n" );
				else
					display.printf( "Searching fix" );
			} else {
				findingFix = true;
				display.print( "." );
			}
			break;

		case GPS::FIX_STATUS::FOUND_FIX:
			if ( findingFix ) {
				display.println();
				findingFix = false;
			}

			display.printf( "Lon/Lat %.3f, %.3f (HDOP %.2f)\r\n", _data.avgLongitude, _data.avgLatitude, _data.HDOP );
			findingFix = false;
			foundFix = true;
			break;
	}
}
