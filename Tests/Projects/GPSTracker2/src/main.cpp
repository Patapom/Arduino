#include <Arduino.h>

// I2C & SPI
#include <SPI.h>

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
//#define TFT_RST		27
//#define TFT_DC		26	// Data Command control pin
//#define TFT_CS		5	// Chip select control pin

#define TFT_RST		33
#define TFT_DC		25	// Data Command control pin
#define TFT_CS		26	// Chip select control pin


#else

#include "../include/User_Setup.h"
#include <TFT_eSPI.h>

TFT_eSPI	tft = TFT_eSPI();

#endif


static const int 		pinMISO = GPIO_NUM_19, pinMOSI = GPIO_NUM_23, pinSCK = GPIO_NUM_18, pinCS = GPIO_NUM_5;	// SD Card pins

//SPIClass	mySPI;

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


void	setup() {
	Serial.begin( 115200 );

	delay( 1000 );

	// =======================================================
	Serial.println( "Initializing SPI..." );

//	mySPI.begin( pinSCK, pinMISO, pinMOSI, pinCS );
	SPI.begin( pinSCK, pinMISO, pinMOSI );


//*	// =======================================================
	Serial.println( "Initializing TFT Screen..." );

Serial.printf( "TFT_RST = %d, TFT_DC = %d, TFT_CS = %d\r\n", TFT_RST, TFT_DC, TFT_CS );

Serial.println("Before init");

#if 1

/*pinMode( TFT_RST, OUTPUT );
digitalWrite(TFT_RST, HIGH);

SPI.transfer(0x11);
delay(120);

// display on
SPI.transfer(0x29);

// write RAM
SPI.transfer(0x2C);

while( 1 );
//*/

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
}

void	loop() {
//	Serial.println( "COUCOU!" );
//	delay( 1000 );
//	return;

	tft.invertDisplay( true );
	tftPrintTest();
	delay( 1000 );

	tft.invertDisplay( false );
	tftPrintTest();
	delay( 1000 );
}
