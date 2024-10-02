#ifndef GLOBAL_H
#define GLOBAL_H

// Define this to output ALL commands/responses to the serial
//#define DEBUG

// Define this to output end product commands/responses to the serial (e.g. client send/receive, server send/receive)
#define DEBUG_LIGHT

// Define this to be the transmitter module (i.e. CLIENT) and specify the transmitter address, undefine to be the receiver module (i.e. SERVER)
//#define TRANSMITTER 1   // Transmitter address is 1

// Default receiver address is the simplest address
#define RECEIVER_ADDRESS  0

// Define this to use the "smart mode" (a.k.a. mode 2)
#define USE_SMART_MODE

#ifdef TRANSMITTER
#if TRANSMITTER == RECEIVER_ADDRESS
  "ERROR! Transmitter cannot use the address reserved for the receiver!"
#endif
#endif

#define LORA_BAUD_RATE  19200 // Can't use too high baud rates with software serial!

static const int  CLIENT_POLL_INTERVAL_MS = 100; // Poll for a command every 100 ms

#define LORA_PIN_RX		2 // RX on D2
#define LORA_PIN_TX		3 // TX on D3
//#define PIN_RESET		4
#define PIN_LED_GREEN	4
#define PIN_LED_RED		5

// HC-SR04 Ultrasound Distance Measurement device
#define PIN_HCSR04_TRIGGER  6
#define PIN_HCSR04_ECHO     7

#define NETWORK_ID  5

#define MAX_MEASUREMENTS  1  // Each measurement takes 8 bytes so we can't store too much on a 2KB Arduino!

typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   byte;
typedef unsigned long   U32;
typedef unsigned short  U16;
typedef unsigned char   U8;


#include <Arduino.h>
#include "Time.h"
#include "HC-SR04.h"
#include "Lora.h"

#include "Commands.h"


#if 1
    static void  __ERROR( bool _setError, const char* _functionName, const char* _message ) {
      if ( !_setError ) return;
// @TODO: Proper error handling
		digitalWrite( PIN_LED_RED, 1 );
		//while ( true ); // Hang... :/
		while ( true ) {
			#ifndef NO_GLOBAL_SERIAL
				Serial.print( "ERROR " );
				Serial.print( _functionName );
				Serial.print( "() => " );
				Serial.println( _message );
				delay( 1000 );
			#else	// Flash instead
				digitalWrite( PIN_LED_RED, _setError ); _setError = !_setError;
				delay( 50 );
			#endif
		}
	}
#else
    static void  __ERROR( bool _setError, const char* _functionName, const char* _message ) {}
#endif
#define	ERROR( _setError, _message ) __ERROR( _setError, __func__, _message )


// Lightweight string class to easily format output
class str {
public:
	char*			m_string;
	static char		ms_globalBuffer[256];
	static char*	ms_globalPointer;
public:
	str( const char* _text, ... ) {
		va_list args;
		va_start( args, _text );	// Arguments pointers is right after our _text argument
		m_string = ms_globalPointer;
		U32 count = vsprintf( m_string, _text, args ) + 1;	// Always count the trailing '\0'!
		ms_globalPointer += count;
		ERROR( U32(ms_globalPointer - ms_globalBuffer) > 256, "Buffer overrun! Fatal error!" );	// Fatal error!
		va_end( args );
	}
	~str() {
		ERROR( m_string >= ms_globalPointer, "Invalid string destruction order! Fatal error!" );  // Fatal error! => Strings should stack and be freed in exact reverse order...
		ms_globalPointer = m_string; // Restore former string pointer
	}

	operator char*() { return m_string; }
};

static void  Log( const char* _header, const char* _text ) {
	Serial.print( _header );
	Serial.println( _text );
}
static void  Log( const char* _text ) { Log( "<LOG> ", _text ); }
static void  LogDebug( const char* _text ) { Log( "<DEBUG> ", _text ); }
static void  LogReply( U16 _commandID, const char* _text ) { Log( str( "%d,<OK> ", _commandID ), _text ); }
static void  LogError( U16 _commandID, const char* _text ) { Log( str( "%d,<ERROR> ", _commandID ), _text ); }

static void  Log() { Log( "" ); }
static void  LogDebug() { LogDebug( "" ); }
static void  LogReply( U16 _commandID ) { LogReply( _commandID, "" ); }
static void  LogError( U16 _commandID ) { LogError( _commandID, "" ); }


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

#endif