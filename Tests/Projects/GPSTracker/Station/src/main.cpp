
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
		display.printf( "Last error → %s\r\n", lora.LastErrorString() );
		display.printf( "Return → %s\r\n", lora.m_receiveBuffer );
		while ( 1 );
	}

//*/

/*	// =======================================================
	display.println( "Initializing GPS..." );

//	Serial1.begin( GPSBaud, SERIAL_8N1, pinRX, pinTX );

	Serial1.begin( GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX );

//	delay( 1000 );

	// Try to find a fix for 15 seconds
	if ( !gps.FindFix( 15000 ) ) {
		display.println( "Initialization failed..." );
		display.println( "Couldn't find any satellite!" );
		while ( 1 );
	}

	display.println( "Awaiting GPS location data..." );
//*/

	startTime_ms = millis();
}

void	loop() {
//	Serial.println( "COUCOU!" );
//	delay( 1000 );
//	return;

//	if ( bno.isFullyCalibrated() )
//		ShowMagnetometerData();

//*	// Wait for LORA messages
	if ( lora.Receive() ) {
		display.printf( "Received %s\r\n", lora.m_receiveBuffer );
	}
//*/
}
