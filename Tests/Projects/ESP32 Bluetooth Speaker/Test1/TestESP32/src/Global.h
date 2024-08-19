#pragma once

#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>

#include <math.h> 
//#include <SPI.h>

// Define this to output to the serial
#define DEBUG

#define PIN_LED_RED   2

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

typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   byte;
typedef unsigned long   U32;
typedef unsigned short  U16;
typedef unsigned char   U8;
typedef          long   S32;
typedef          short  S16;
typedef          char   S8;

#if 1
    static void  ERROR( bool _setError, const char* _functionName, const char* _message ) {
      if ( !_setError ) return;
// @TODO: Error handling
digitalWrite( PIN_LED_RED, 1 );
//while ( true ); // Hang... :/
while ( true ) {
  Serial.print( "ERROR " );
  Serial.print( _functionName );
  Serial.print( "() => " );
  Serial.println( _message );
  delay( 1000 );
}
    }
#else
    static void  ERROR( bool _setError, const char* _functionName, const char* _message ) {}
#endif

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
    ERROR( U32(ms_globalPointer - ms_globalBuffer) > 256, "str", "Buffer overrun! Fatal error!" ); // Fatal error!
    va_end( args );
  }
  ~str() {
    ERROR( m_string >= ms_globalPointer, "~str", "Invalid string destruction order! Fatal error!" );  // Fatal error! => Strings should stack and be freed in exact reverse order...
    ms_globalPointer = m_string; // Restore former string pointer
  }

  operator char*() { return m_string; }
};

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