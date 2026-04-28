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

// I2C & SPI
#include <Wire.h>
#include <SPI.h>

// File system & SD card libraries
#include <SD.h>
#include <FS.h>

// TFT screen (ST7789 v3.0)
#if 1
#include <Adafruit_GFX.h>		// Core graphics library
//#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h>	// Hardware-specific library for ST7789

//#define TFT_MOSI 23  // SDA Pin on ESP32
//#define TFT_SCLK 18  // SCL Pin on ESP32
//#define TFT_CS   15  // Chip select control pin
//#define TFT_DC    2  // Data Command control pin
//#define TFT_RST   4  // Reset pin (could connect to RST pin)

#define TFT_MOSI	23	// SDA Pin on ESP32
#define TFT_SCLK	18	// SCL Pin on ESP32

//#define TFT_RST		-1	// No hardware reset
//#define TFT_RST		14	// Conflict SPI
#define TFT_RST		33
#define TFT_DC		25	// Data Command control pin
#define TFT_CS		26	// Chip select control pin
//#define TFT_CS		-1	// Chip select control pin

#else
#include "../include/User_Setup.h"
#include <TFT_eSPI.h>
#endif

// GPS Module (Neo 6M)
#include <TinyGPSPlus.h>

// Magnetometer (BNO 055)
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


TinyGPSPlus		GPS;
static const uint32_t	GPSBaud = 9600;
static const uint32_t	LORABaud = 115200;

Adafruit_BNO055	bno = Adafruit_BNO055( 55, 0x29 );

#if 1	// ESP32
static const int 		pinMISO = GPIO_NUM_19, pinMOSI = GPIO_NUM_23, pinSCK = GPIO_NUM_18, pinCS = GPIO_NUM_5;	// SD Card pins
static const int 		pinRX = GPIO_NUM_16, pinTX = GPIO_NUM_17;	// GPS Communication & Decoding
static const int		pinSDA = GPIO_NUM_21, pinSCL = GPIO_NUM_22;	// I2C for accelerometer
static const int 		pinRX2 = GPIO_NUM_35, pinTX2 = GPIO_NUM_32;	// RYLR998 Lora Communication
#else	// XIAO
//static const int 		pinMISO = D9, pinMOSI = D10, pinSCK = D8, pinCS = D2;
static const int 		pinMISO = GPIO_NUM_9, pinMOSI = GPIO_NUM_10, pinSCK = GPIO_NUM_8, pinCS = GPIO_NUM_4;	// SD Card pins
static const int 		pinRX = GPIO_NUM_0, pinTX = GPIO_NUM_1;		// GPS Communication & Decoding
static const int		pinSDA = , pinSCL = ;
#endif

//SPIClass	mySPI;
//TFT_eSPI	tft = TFT_eSPI();

void	ShowGPSData();
void	ShowMagnetometerData();
void	GetCalibration();
bool	ShouldCalibrate();
void	Calibrate();
void	ScanI2C();

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789	tft = Adafruit_ST7789( TFT_CS, TFT_DC, TFT_RST );
 
float p = 3.1415926;
 
void tftPrintTest() {
	tft.setTextWrap(false);
	tft.fillScreen(ST77XX_BLACK);
	tft.setCursor(0, 30);
	tft.setTextColor(ST77XX_RED);
	tft.setTextSize(1);
	tft.println("Hello World!");
	tft.setTextColor(ST77XX_YELLOW);
	tft.setTextSize(2);
	tft.println("Hello World!");
	tft.setTextColor(ST77XX_GREEN);
	tft.setTextSize(3);
	tft.println("Hello World!");
	tft.setTextColor(ST77XX_BLUE);
	tft.setTextSize(4);
	tft.print(1234.567);

	delay(1500);

	tft.setCursor(0, 0);
	tft.fillScreen(ST77XX_BLACK);
	tft.setTextColor(ST77XX_WHITE);
	tft.setTextSize(0);
	tft.println("Hello World!");
	tft.setTextSize(1);
	tft.setTextColor(ST77XX_GREEN);
	tft.print(p, 6);
	tft.println(" Want pi?");
	tft.println(" ");
	tft.print(8675309, HEX); // print 8,675,309 out in HEX!
	tft.println(" Print HEX!");
	tft.println(" ");
	tft.setTextColor(ST77XX_WHITE);
	tft.println("Sketch has been");
	tft.println("running for: ");
	tft.setTextColor(ST77XX_MAGENTA);
	tft.print(millis() / 1000);
	tft.setTextColor(ST77XX_WHITE);
	tft.print(" seconds.");
}

void	TestTime() {
	struct tm tm_utc = {0};
	tm_utc.tm_year = 2025 - 1900;
	tm_utc.tm_mon  = 3 - 1;
	tm_utc.tm_mday = 10;
	tm_utc.tm_hour = 12;
	tm_utc.tm_min  = 0;
	tm_utc.tm_sec  = 0;

	// Convertir en time_t (interprété comme UTC)
//	time_t	t = timegm( &tm_utc );  // GNU / POSIX
//	time_t	t = _mkgmtime( &tm_utc );  // GNU / POSIX
	time_t	t = mktime( &tm_utc );

	// Définir le fuseau PST avec DST automatique
	setenv( "TZ", "America/Los_Angeles", 1 );
	setenv( "TZ", "PST8PDT,M3.2.0,M11.1.0", 1 );
	tzset();

	struct tm*	tm_pst = localtime( &t );

	Serial.printf( "Heure PST/PDT: %s", asctime(tm_pst) );
}

void	setup() {
	Serial.begin( 115200 );
//	while ( !Serial.isConnected() ) {
//		delay( 100 );
//	}

	delay( 1000 );

TestTime();

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

//	digitalWrite( TFT_CS, LOW );

Serial.printf( "TFT_RST = %d, TFT_DC = %d, TFT_CS = %d\r\n", TFT_RST, TFT_DC, TFT_CS );

//pinMode( TFT_BL, OUTPUT );
//digitalWrite( TFT_BL, HIGH );

Serial.println("Before init");

#if 1

//pinMode( TFT_RST, OUTPUT );
//digitalWrite(TFT_RST, HIGH);
//
//SPI.transfer(0x11);
//delay(120);
//
//// display on
//SPI.transfer(0x29);
//
//// write RAM
//SPI.transfer(0x2C);
//
//while( 1);
//
//
//digitalWrite(TFT_RST, LOW);
//delay(50);
//digitalWrite(TFT_RST, HIGH);
//delay(50);


//pinMode(25, OUTPUT);
//
//digitalWrite(25, LOW);
//delay(500);
//digitalWrite(25, HIGH);
//delay(500);


	tft.init( 240, 280, SPI_MODE0 );	// Init ST7789 display 135x240 pixel
	tft.setRotation( 2 );
//	tft.fillScreen( ST77XX_BLACK );
	tft.fillScreen( ST77XX_RED );

/*
digitalWrite( TFT_CS, LOW );

// Commande RAM write
digitalWrite( TFT_DC, LOW );
SPI.transfer( 0x2C );

// Données
digitalWrite( TFT_DC, HIGH );
for (int i = 0; i < 240 * 240; i++) {
	SPI.transfer( 0xF8 );	// rouge
	SPI.transfer( 0x00 );
}

digitalWrite( TFT_CS, HIGH );
*/

//while ( 1 );

#else
	tft.init();

	#if 1
	tft.setRotation( 0 );
	tft.fillScreen( TFT_RED );
	#else
	tft.setRotation( 1 );
	
	tft.fillScreen( TFT_BLACK );
	tft.setTextColor( TFT_WHITE );
	tft.setTextSize( 2 );

	tft.setCursor( 10, 10 );
	tft.println( "ESP32 ST7789 OK" );
//	tft.drawPixel( 10, 10, TFT_WHITE );
	#endif

#endif

Serial.println("After init");
//*/

	digitalWrite( TFT_CS, HIGH );

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

	if ( ShouldCalibrate() ) {
		Serial.println( "Entering calibration mode..." );
		Calibrate();
	}
//*/


	// =======================================================
	Serial.println( "Initializing LORA module..." );

	Serial2.begin( LORABaud, SERIAL_8N1, pinRX2, pinTX2 );
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


	// =======================================================
	Serial.println( "Initializing GPS module..." );

	Serial1.begin( GPSBaud, SERIAL_8N1, pinRX, pinTX );

	delay( 1000 );

//	// Vancouver Time Zone
//	setenv( "TZ", "PST8PDT,M3.2.0,M11.1.0", 1 );
//	tzset();

	Serial.println( "Awaiting GPS location data..." );
}

bool	foundFix = false;
bool	findingFix = false;

void	loop() {
//	Serial.println( "COUCOU!" );
//	delay( 1000 );
//	return;

#if 0
//	digitalWrite( TFT_CS, LOW );

	tft.invertDisplay( true );
	tftPrintTest();
	delay( 1000 );

	tft.invertDisplay( false );
	tftPrintTest();
	delay( 1000 );

//return;
#endif

//	if ( bno.isFullyCalibrated() )
//		ShowMagnetometerData();

#if 0 // Basic serial printing of GPS data
	if ( Serial1.available() ) {
		Serial.print( (char) Serial1.read() );
	}
	return;
#endif

	if ( Serial1.available() == 0 || !GPS.encode( Serial1.read() ) ) {
		if ( millis() > 5000 && GPS.charsProcessed() < 10 ) {
			Serial.println( F("No GPS detected: check wiring.") );
			while(true);
		}
		delay( 100 );
		return;
	}

	ShowGPSData();
	delay( 1000 );
}

void	ShowMagnetometerData() {
	// Lecture des angles (Euler)
	imu::Vector<3> euler = bno.getVector( Adafruit_BNO055::VECTOR_EULER );

	float	heading = euler.x(); // Cap (boussole)
	float	roll    = euler.z();
	float	pitch   = euler.y();

	float 	bearing = bno.getTemp();
	int 	temp = bno.getTemp();

	Serial.print( "Cap: " );
	Serial.print( heading );
	Serial.print( "°  | Pitch: " );
	Serial.print( pitch );
	Serial.print( "°  | Roll: " );
	Serial.print( roll );
	Serial.print( "°" );
	Serial.printf( " | Bearing: %.2f° | Temp: %d°C\r\n", bearing, temp );
}

void	GetCalibration() {

	// Show mode
	adafruit_bno055_opmode_t	operatingMode = bno.getMode();
	const char*					strMode = NULL;
	switch ( operatingMode ) {
  		case OPERATION_MODE_CONFIG:		strMode = "OPERATION_MODE_CONFIG"; break;
  		case OPERATION_MODE_ACCONLY:	strMode = "OPERATION_MODE_ACCONLY"; break;
  		case OPERATION_MODE_MAGONLY: 	strMode = "OPERATION_MODE_MAGONLY"; break;
  		case OPERATION_MODE_GYRONLY: 	strMode = "OPERATION_MODE_GYRONLY"; break;
  		case OPERATION_MODE_ACCMAG: 	strMode = "OPERATION_MODE_ACCMAG"; break;
  		case OPERATION_MODE_ACCGYRO: 	strMode = "OPERATION_MODE_ACCGYRO"; break;
  		case OPERATION_MODE_MAGGYRO:	strMode = "OPERATION_MODE_MAGGYRO"; break;
  		case OPERATION_MODE_AMG: 		strMode = "OPERATION_MODE_AMG"; break;
  		case OPERATION_MODE_IMUPLUS:	strMode = "OPERATION_MODE_IMUPLUS"; break;
  		case OPERATION_MODE_COMPASS:	strMode = "OPERATION_MODE_COMPASS"; break;
  		case OPERATION_MODE_M4G: 		strMode = "OPERATION_MODE_M4G"; break;
  		case OPERATION_MODE_NDOF_FMC_OFF: strMode = "OPERATION_MODE_NDOF_FMC_OFF"; break;
  		case OPERATION_MODE_NDOF: 		strMode = "OPERATION_MODE_NDOF"; break;
	}
	Serial.printf( "BNO operating mode: %s\r\n", strMode );

	// Show calibration values
	uint8_t sys, gyro, accel, mag;
	bno.getCalibration( &sys, &gyro, &accel, &mag );

	Serial.print("CALIB SYS:");
	Serial.print(sys);
	Serial.print(" G:");
	Serial.print(gyro);
	Serial.print(" A:");
	Serial.print(accel);
	Serial.print(" M:");
	Serial.println(mag);
/// valeurs de 0 à 3
/// pour une bonne boussole → mag = 3
}

bool	ShouldCalibrate() {
	uint8_t sys, gyro, accel, mag;
	bno.getCalibration( &sys, &gyro, &accel, &mag );

	return mag < 3;
}

void	Calibrate() {
	uint8_t sys, gyro, accel, mag = 0;

	while ( mag < 3 ) {
		bno.getCalibration( &sys, &gyro, &accel, &mag );
		Serial.printf( "Calibrating... sys=%d gyro=%d accel=%d mag=%d\r\n", sys, gyro, accel, mag );
		delay( 250 );
	}

	// Read offsets (store for next time)
	adafruit_bno055_offsets_t	offsets;
	bno.getSensorOffsets( offsets );
//	bno.setSensorOffsets( offsets );
	Serial.printf( "Final offsets: \r\n" );
	Serial.printf( " → Accel = %d, %d, %d (radius = %d)\r\n", offsets.accel_offset_x, offsets.accel_offset_y, offsets.accel_offset_z, offsets.accel_radius );
	Serial.printf( " → Mag   = %d, %d, %d (radius = %d)\r\n", offsets.mag_offset_x, offsets.mag_offset_y, offsets.mag_offset_z, offsets.mag_radius );
	Serial.printf( " → Gyro  = %d, %d, %d\r\n", offsets.gyro_offset_x, offsets.gyro_offset_y, offsets.gyro_offset_z );

// Final offsets: 
//  → Accel = 16379, 8652, 16379 (radius = 7440)
//  → Mag   = 29597, -32755, 8800 (radius = 16380)
//  → Gyro  = 16379, 8088, 16380
}

// Restores the offsets found by the calibration
void	RestoreOffsets() {
//  → Accel = 16379, 8652, 16379 (radius = 7440)
//  → Mag   = 29597, -32755, 8800 (radius = 16380)
//  → Gyro  = 16379, 8088, 16380
	adafruit_bno055_offsets_t	offsets;
	offsets.accel_offset_x = 16379, 8652, 16379;
	offsets.accel_offset_y = 16379, 8652, 16379;
	offsets.accel_offset_z = 16379, 8652, 16379;
	offsets.accel_radius = 16379, 8652, 16379;
	offsets.gyro_offset_x = 16379, 8652, 16379;

	bno.getSensorOffsets( offsets );
}

void	ShowGPSDateTime();

void	ShowGPSData() {

	ShowGPSDateTime();

	if ( !GPS.location.isValid() ) {
		if ( findingFix ) {
			Serial.print( "." );
		} else {
			findingFix = true;
			if ( !foundFix ) {
				Serial.print( "Invalid GPS position → Waiting for a fix" );
			} else {
				Serial.print( "Lost fix → Waiting for a fix" );
			}
		}

		return;
	}

	foundFix = true;
	if ( findingFix ) {
		Serial.println( " FOUND!" );
		findingFix = false;
	}

	if ( GPS.location.isValid() ) {
		Serial.print( "> Location " );
		Serial.print( GPS.location.lat(), 6 );
		Serial.print( ", " );
		Serial.print( GPS.location.lng(), 6 );
		Serial.println();
	}
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


//	// UTC time zone
//	setenv( "TZ", "UTC0", 1 );
//	tzset();
//	time_t	utc = mktime( &t );

	// Vancouver Time Zone
	setenv( "TZ", "PST8PDT,M3.2.0,M11.1.0", 1 );
	tzset();

	// Convert into timestamp
//	time_t		t = timegm( &t );	// Non standard... Doesn't exist on ESP32
	time_t		t = mktime( &utc );
	struct tm*	local = localtime( &t );

	// UTC
	Serial.printf( "> UTC Date %04d/%02d/%02d\r\n", GPS.date.year(), GPS.date.month(), GPS.date.day() );
	Serial.printf( "> UTC Time %02d:%02d:%02d\r\n", GPS.time.hour(), GPS.time.minute(), GPS.time.second() );

	// Local
	Serial.printf( "> Date %04d/%02d/%02d\r\n", local->tm_year, local->tm_mon, local->tm_mday );
	Serial.printf( "> Time %02d:%02d:%02d\r\n", local->tm_hour, local->tm_min, local->tm_sec );
}

void	ScanI2C() {
	Serial.println( "Scan I2C..." );
	bool	found = false;
	for ( byte addr=1; addr < 127; addr++ ) {
		Wire.beginTransmission( addr );
		int	error = Wire.endTransmission();
		if ( error == 0 ) {
			found = true;	// Found at least 1 device!

			Serial.print( "Device found at 0x" );
			Serial.println( addr, HEX );
		} else if (error==4) {
			Serial.print("Unknow error at address 0x");
			if ( addr  < 16 ) {
				Serial.print( "0" );
			}
			Serial.println( addr, HEX );
		}
	}
	if ( !found )
		Serial.println( "Scan failed: No device found!" );
}