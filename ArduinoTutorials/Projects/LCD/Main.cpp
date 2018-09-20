// Lesson 22 + A piece of lesson 33 using the rotary encoder

//*
//  LiquidCrystal Library - Hello World
//
// Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
// library works with all LCD displays that are compatible with the
// Hitachi HD44780 driver. There are many of them out there, and you
// can usually tell them by the 16-pin interface.
//
// This sketch prints "Hello World!" to the LCD
// and shows the time.
//
//  The circuit:
// * LCD RS pin to digital pin 7
// * LCD Enable pin to digital pin 8
// * LCD D4 pin to digital pin 9
// * LCD D5 pin to digital pin 10
// * LCD D6 pin to digital pin 11
// * LCD D7 pin to digital pin 12
// * LCD R/W pin to ground
// * LCD VSS pin to ground
// * LCD VCC pin to 5V
// * 10K resistor:
// * ends to +5V and ground
// * wiper to LCD VO pin (pin 3)
//
// Library originally added 18 Apr 2008
// by David A. Mellis
// library modified 5 Jul 2009
// by Limor Fried (http://www.ladyada.net)
// example added 9 Jul 2009
// by Tom Igoe
// modified 22 Nov 2010
// by Tom Igoe
//
// This example code is in the public domain.
//
// http://www.arduino.cc/en/Tutorial/LiquidCrystal
//

// include the library code:
#include <LiquidCrystal.h>
#include "Pom/Pom.h"

// initialize the library with the numbers of the interface pins
LiquidCrystal	lcd( 7, 8, 9, 10, 11, 12 );

#define	ROT_PIN_A		2	// Clock
#define	ROT_PIN_B		3	// DT
#define	ROT_PIN_SWITCH	4	// Switch => Doesn't work very well, it randomly switches on and off when rotating the shaft + clicking on it will stay clicked for a long time before coming down again! :'(

bool	lastSwitch = true;
U8		switchCounter = 0;
U8		lastStateA = false;
U8		lastStateB = false;
S32		counter = 0;

void setup2() {
	lcd.begin(16, 2);			// set up the LCD's number of columns and rows:
	lcd.print("Hello, World!");	// Print a message to the LCD.

	pinMode( ROT_PIN_A, INPUT );
	pinMode( ROT_PIN_B, INPUT );
	pinMode( ROT_PIN_SWITCH, INPUT );
	lastStateA = digitalRead( ROT_PIN_A ) == LOW;
	lastStateB = digitalRead( ROT_PIN_B ) == LOW;
}

void loop() {
	// Handle rotation
	U8	valA = digitalRead( ROT_PIN_A ) == LOW;
	U8	valB = digitalRead( ROT_PIN_B ) == LOW;
// 	if ( valA != lastStateA || valB != lastStateB ) {
// 		// Rotating!
// 		lastStateA = valA;
// 		lastStateB = valB;
// 		if ( valB ) {
// 			counter--;	// B changed first -> CCW
// 		} else {
// 			counter++;	// A changed first -> CW
// 		}
// 
// 		Serial.print( "A = " );
// 		Serial.print( valA );
// 		Serial.print( " - B = " );
// 		Serial.print( valB );
// 		Serial.println();
// 	}

	// Simply detect a raising edge and check if B is already pushed or not...
	if ( valA && !lastStateA ) {
		if ( valB )
			counter++;	// B changed first -> CW
		else
			counter--;	// A changed first -> CCW
	}
	lastStateA = valA;

// Doesn't work very well, it randomly switches on and off when rotating the shaft + clicking on it will stay clicked for a long time before coming down again! :'(
//  	// Handle switch click
//  	bool	sw = digitalRead( ROT_PIN_SWITCH ) == LOW;
//  	if ( sw && !lastSwitch ) {
//  		lcd.setCursor( 0, 0 );
//  		lcd.print( "CLICK!               " );
//  
//  	} else if ( !sw && lastSwitch ) {
//  		lcd.setCursor( 0, 0 );
//  		lcd.print( "Hello, World!        " );
//  	}
//  	lastSwitch = sw;

//  	Serial.print( digitalRead( ROT_PIN_SWITCH ) );
//  	Serial.print(  " - last = " );
//  	Serial.print(  lastSwitch );
//  	Serial.print(  " - count = " );
//  	Serial.println( switchCounter );

	// I made a "buffered read" function that only returns true if the state equals the expected value for a given amount of time
 	bool	sw = BufferedRead( ROT_PIN_SWITCH, lastSwitch ? LOW : HIGH, 8, switchCounter );	// Expect inverse state
 	if ( sw ) {
		if ( lastSwitch ) {
			// Going from HIGH to LOW => Clicked!
 			lcd.setCursor( 0, 0 );
 			lcd.print( "CLICK!               " );
 		} else {
			// Going from LOW to HIGH => Released!
 			lcd.setCursor( 0, 0 );
 			lcd.print( "Hello, World!        " );
 		}
	 	lastSwitch = !lastSwitch;
	}

	lcd.setCursor( 0, 1 );			// set the cursor to column 0, line 1 (note: line 1 is the second row, since counting begins with 0):
//	lcd.print( millis() / 1000 );	// print the number of seconds since reset:
	lcd.print( counter );			// Print rotary counter
	lcd.print( "              " );
}
//*/