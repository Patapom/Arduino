#ifndef GLOBAL_H
#define GLOBAL_H

#include <SPI.h>
#include <nRF24L01.h>

// Define this to output to the serial
#define DEBUG

#define PIN_IRQ   2 // IRQ
#define PIN_CE    7 // Chip Enable
#define PIN_CSN   8 // Chip Select

// SPI
#define PIN_MOSI  11  // Input
#define PIN_MISO  12  // Output
#define PIN_SCK   13

#define PIN_LED_RED   6

typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   byte;
typedef unsigned long   U32;
typedef unsigned short  U16;
typedef unsigned char   U8;

static char s_globalBuffer[256];

#if 1
    void  ERROR( bool _setError, char* _functionName, char* _message ) {
      if ( !_setError ) return;
// @TODO: Error handling
digitalWrite( PIN_LED_RED, 1 );
while ( true ); // Hang... :/
    }
#else
    void  ERROR( bool _setError, char* _functionName, char* _message ) {}
#endif

class str {
  char*         m_string;
  static char*  ms_globalBuffer;
public:
  str( char* _text, ... ) {
    va_list args;
    va_start( args, _text );  // Arguments pointers is right after our _text argument
    m_string = ms_globalBuffer;
    int count = vsprintf( m_string, _text, args ) + 1;  // Always count the trailing '\0'!
    ms_globalBuffer += count;
    ERROR( (ms_globalBuffer - s_globalBuffer) > 256, "str", "Buffer overrun! Fatal error!" ); // Fatal error!
    va_end( args );
  }
  ~str() {
    ERROR( m_string >= ms_globalBuffer, "~str", "Invalid string destruction order! Fatal error!" );  // Fatal error! => Strings should stack and be freed in exact reverse order...
    ms_globalBuffer = m_string; // Restore former string pointer
  }

  operator char*() { return m_string; }
};
/*
void  Log( char* _header, char* _text ) {
  Serial.print( _header );
  Serial.println( _text );
}
void  Log( char* _text ) { Log( "<LOG> ", _text ); }
void  LogDebug( char* _text ) { Log( "<DEBUG> ", _text ); }
void  LogReply( U16 _commandID, char* _text ) { Log( str( "%d,<OK> ", _commandID ), _text ); }
void  LogError( U16 _commandID, char* _text ) { Log( str( "%d,<ERROR> ", _commandID ), _text ); }

void  Log() { Log( "" ); }
void  LogDebug() { LogDebug( "" ); }
void  LogReply( U16 _commandID ) { LogReply( _commandID, "" ); }
void  LogError( U16 _commandID ) { LogError( _commandID, "" ); }
*/
void Flash( int _pin, int _duration_ms, int _count ) {
  for ( int i=0; i < _count; i++ ) {
    digitalWrite( _pin, HIGH );
    delay( _duration_ms );
    digitalWrite( _pin, LOW );
    if ( i != _count-1 )
      delay( _duration_ms );
  }
}
void Flash( int _duration_ms ) {
  Flash( PIN_LED_RED, _duration_ms, 1 );
}
void Flash( int _duration_ms, int _count ) {
  Flash( PIN_LED_RED, _duration_ms, _count );
}

#endif