#include "Global.h"


char	str::ms_globalBuffer[256];
char* 	str::ms_globalPointer = str::ms_globalBuffer;

/*str::str( const char* _text, ... ) {
	va_list args;
	va_start( args, _text );	// Arguments pointers is right after our _text argument
	m_string = ms_globalPointer;
	U32 count = vsprintf( m_string, _text, args ) + 1;	// Always count the trailing '\0'!
	ms_globalPointer += count;
	ERROR( U32(ms_globalPointer - ms_globalBuffer) > 256, "str buffer overrun! Fatal error!" );	// Fatal error!
	va_end( args );
}
*/
str::str( FChar* _text, ... ) {
	va_list args;
	va_start( args, _text );	// Arguments pointers is right after our _text argument
	m_string = ms_globalPointer;
	U32 count = vsprintf_P( m_string, (const char*) _text, args ) + 1;	// Always count the trailing '\0'!
	ms_globalPointer += count;
	ERROR( U32(ms_globalPointer - ms_globalBuffer) > 256, "str buffer overrun! Fatal error!" );	// Fatal error!
	va_end( args );
}
