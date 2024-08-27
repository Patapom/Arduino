#pragma once

#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>
#include <math.h>

#include "xtensa/core-macros.h"	// XTHAL_GET_CCOUNT()

// Define this to output to the serial
#define DEBUG

#define PIN_LED_RED   2 // Onboard LED is connected to GPIO2 -> Used to signal an error

typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   byte;
typedef unsigned long   U32;
typedef unsigned long long  U64;
typedef unsigned short  U16;
typedef unsigned char   U8;
typedef          long long S64;
typedef          long   S32;
typedef          short  S16;
typedef          char   S8;

template<typename T>
static T	clamp( T x, T min, T max ) { return x < min ? min : x > max ? max : x; }
template<typename T>
static T	saturate( T x ) { return x < 0 ? 0 : x > 1 ? 1 : x; }

// SPI
// On my board, writing the pin numbers yields:
// MOSI: 23
// MISO: 19
// SCK: 18
// SS: 5
#define PIN_MOSI  MOSI  // Input
#define PIN_MISO  MISO  // Output
#define PIN_SCK   SCK   // Signal clock
#define PIN_CSN   SS    // Chip Select

extern S16	gs_sine[16384];
static void	InitSine() {
	for ( U32 i=0; i < 16384; i++ )
		gs_sine[i] = 32767 * sin( 2 * PI * i / 16384.0f );
}
static S16	FastSine( U32 i ) { return gs_sine[i & 0x3FFF]; }

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
//    static void  ERROR( bool _setError, const char* _message ) {
//		ERROR( _setError, __func__, _message );
//	}

class str {
public:
  char*         m_string;
  static char   ms_globalBuffer[256];
  static char*  ms_globalPointer;
public:
  str( const char* _text, ... ) {
    va_list args;
    va_start( args, _text );  // Arguments pointers is right after our _text argument
    m_string = ms_globalPointer;
    U32 count = vsprintf( m_string, _text, args ) + 1;  // Always count the trailing '\0'!
    ms_globalPointer += count;
    ERROR( U32(ms_globalPointer - ms_globalBuffer) > 256, "Buffer overrun! Fatal error!" ); // Fatal error!
    va_end( args );
  }
  ~str() {
    ERROR( m_string >= ms_globalPointer, "Invalid string destruction order! Fatal error!" );  // Fatal error! => Strings should stack and be freed in exact reverse order...
    ms_globalPointer = m_string; // Restore former string pointer
  }

  operator char*() { return m_string; }
};

static void	ESP_ERROR( esp_err_t _error, const char* _message ) {
	if ( _error == ESP_OK )
		return;
	char	message[512];
	ERROR( true, str( "%s (%s)", _message, esp_err_to_name_r( _error, message, 512 ) ) );
}

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