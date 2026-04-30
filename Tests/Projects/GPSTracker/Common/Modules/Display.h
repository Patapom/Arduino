#pragma once

#include "Global.h"
#include <Adafruit_ST7789.h>	// Hardware-specific library for ST7789

class Adafruit_ST7789;

class	TFTDisplay {
public:
	Adafruit_ST7789&	m_tft;

	U16		m_backColor;

public:
	TFTDisplay( Adafruit_ST7789& _tft ) : m_tft( _tft ) {
		m_backColor = 0;
	}

	void	Clear() { Clear( m_backColor ); }
	void	Clear( U16 RGB );
	void	Clear( U8 R, U8 G, U8 B ) { Clear( RGB16( R, G, B ) ); }

	void	SetTextProperties( U8 _size, U16 _cursorX, U16 _cursorY, U16 _color );

	void	print( const char* _text );
	void	println( const char* _text );
	void	printf( const char* _text, ... );

	void 	PrintTest();
	void 	PrintTest2();

	static U16	RGB16( U8 R, U8 G, U8 B ) { return (B & 0x1F) | ((G & 0x3F) << 5) | ((R & 0x1F) << 11); }
};