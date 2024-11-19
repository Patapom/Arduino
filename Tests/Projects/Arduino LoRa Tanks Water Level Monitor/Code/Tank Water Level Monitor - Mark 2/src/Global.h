#ifndef GLOBAL_H
#define GLOBAL_H

// Define this to output ALL commands/responses to the serial
//#define DEBUG

// Define this to output end product commands/responses to the serial (e.g. client send/receive, server send/receive)
#define DEBUG_LIGHT

// Define this to be the transmitter module (i.e. MONITOR) and specify the transmitter address, undefine to be the receiver module (i.e. LISTENER)
//#define MONITOR

// Default receiver address is the simplest address
#define RECEIVER_ADDRESS  0

// Define this to use the "smart mode" (a.k.a. mode 2)
//#define USE_SMART_MODE

#if defined(MONITOR) && MONITOR == RECEIVER_ADDRESS
  "ERROR! Transmitter cannot use the address reserved for the receiver!"
#endif

// Remember that if you change this, you have to reconfigure the LoRa chips with the USB tool to make it go from 19200 to whatever baud rate you choose
#define LORA_BAUD_RATE  19200	// Can't use too high baud rates with software serial!

//#define LORA_CONFIG		915000000, 9, 7, 1, 12	// Default (okay for small messages < 100 bytes)
//#define LORA_CONFIG		915000000, 8, 7, 1, 12	// Recommended when payload > 100 bytes
#define LORA_CONFIG		915000000, 5, 9, 1, 4	// Smallest spread factor for larger messages

// All LoRa devices need to use the same network
#define NETWORK_ID  5


#define PIN_LORA_RX		2 // RX on D2
#define PIN_LORA_TX		3 // TX on D3
//#define PIN_RESET		4
#define PIN_LED_GREEN	4
#define PIN_LED_RED		5

typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   byte;
typedef unsigned long   U32;
typedef unsigned short  U16;
typedef unsigned char   U8;
typedef signed long   	S32;
typedef signed short  	S16;
typedef signed char   	S8;


#include <Arduino.h>

typedef const __FlashStringHelper	FChar;

#include "./Helpers/Time.h"
#include "./Helpers/HC-SR04.h"
#include "./Helpers/Lora.h"
#include "./Helpers/Commands.h"


#if 1
    static void  __ERROR( bool _setError, const char* _functionName, FChar* _message ) {
      if ( !_setError ) return;
// @TODO: Proper error handling
		digitalWrite( PIN_LED_RED, 1 );
		//while ( true ); // Hang... :/
		while ( true ) {
			#ifndef NO_GLOBAL_SERIAL
				Serial.print( F("ERROR ") );
				Serial.print( _functionName );
				Serial.print( F("() => ") );
				Serial.println( _message );
				delay( 1000 );
			#else	// Flash instead
				digitalWrite( PIN_LED_RED, _setError ); _setError = !_setError;
				delay( 50 );
			#endif
		}
	}
#else
    static void  __ERROR( bool _setError, const __FlashStringHelper* _functionName, const __FlashStringHelper* _message ) {}
#endif
#define	ERROR( _setError, _message ) __ERROR( _setError, __func__, F(_message) )


// Lightweight string class to easily format output
class str {
public:
	char*			m_string;
	static char		ms_globalBuffer[256];
	static char*	ms_globalPointer;

public:
//	str( const char* _text, ... );
	str( FChar* _text, ... );
	~str() {
		ERROR( m_string >= ms_globalPointer, "Invalid string destruction order! Fatal error!" );  // Fatal error! => Strings should stack and be freed in exact reverse order...
		ms_globalPointer = m_string; // Restore former string pointer
	}

	operator const char*() { return m_string; }
};

static void  Log( const char* _header, const char* _text ) {
	Serial.print( _header );
	Serial.println( _text );
}

static void  Log( const char* _text ) { Log( str( F( "<LOG> " ) ), _text ); }
static void  LogDebug( const char* _text ) { Log( str( F( "<DEBUG> " ) ), _text ); }
static void  LogReply( U16 _commandID, const char* _text ) { Log( str( F( "%d,<OK> " ), _commandID ), _text ); }
static void  LogError( U16 _commandID, const char* _text ) { Log( str( F( "%d,<ERROR> " ), _commandID ), _text ); }

static void  Log() { Log( str( F("") ) ); }
static void  LogDebug() { LogDebug( str( F("") ) ); }
static void  LogReply( U16 _commandID ) { LogReply( _commandID, str( F("") ) ); }
static void  LogError( U16 _commandID ) { LogError( _commandID, str( F("") ) ); }


static void Flash( int _pin, int _duration_ms, int _count ) {
  for ( int i=0; i < _count; i++ ) {
    digitalWrite( _pin, HIGH );
    delay( _duration_ms );
    digitalWrite( _pin, LOW );
    if ( i != _count-1 )
      delay( _duration_ms );
  }
}

static void Flash( int _duration_ms ) {
  Flash( PIN_LED_RED, _duration_ms, 1 );
}
static void Flash( int _duration_ms, int _count ) {
  Flash( PIN_LED_RED, _duration_ms, _count );
}

// A structure storing a single distance measurement from the HCSR04 device
struct Measurement {
	U16		time_s;				// Time of measurement, in seconds. Either relative to the start time (monitor side), or relative to the first measurement sent (listener side)
	U16		rawValue_micros;	// The raw time of flight from the sensor, in µseconds

	float	GetDistance() { return ConvertTimeOfFlightToDistance( rawValue_micros ); }
};

#endif
