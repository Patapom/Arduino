#include "Pom.h"

U32	SerialPrintf( char *str, ... ) {
	U32		count=0, j=0;
	char	temp[ARDBUFFER+1];

	// Count # of arguments
	for ( U32 i=0; str[i] != '\0'; i++ )
		if ( str[i] == '%' )
			count++;

	va_list	argv;
	va_start( argv, str );

	U8	c = ' ';
	for( j=0; c != '\0'; ) {
		c = *str++;
		if ( c == '%' ) {
			// Print existing buffer as string and reset
			temp[j] = '\0';
			Serial.print( temp );
			j=0;
			temp[0] = '\0';

			// Handle argument
			switch( *str++ ) {
				case 'c': Serial.print( (char) va_arg( argv, int ) ); break;
				case 'd': Serial.print( va_arg( argv, int ) ); break;
				case 'x': Serial.print( va_arg( argv, int ), HEX ); break;
				case 'l': Serial.print( va_arg( argv, long ) ); break;
				case 'f': Serial.print( va_arg( argv, double ), 8 ); break;
				case 's': Serial.print( va_arg( argv, char* ) ); break;
			};
		} else {
			// Append a regular character
			temp[j++] = c;
			if ( c == '\0' || c == '\n' || j == ARDBUFFER ) {
				temp[j] = '\0';
				if ( c == '\n' ) {
					temp[--j] = '\0';		// Erase the '\n' we just wrote before
					Serial.println( temp );	// Line feed
				} else {
					Serial.print( temp );	// Regular print
				}
				j = 0;
				temp[0] = '\0';
			}
		}
	}

	return count + 1;
}