#pragma once

#include "../Global.h"

//#include <Adafruit_ST7789.h>	// Hardware-specific library for ST7789
#include <TFT_eSPI.h>

#include "../Helpers/BMPFile.h"	// Bitmap class

class	TFTDisplay {
public:
//	Adafruit_ST7789&	m_tft;
	TFT_eSPI&			m_tft;

	U16		m_backColor;

public:
	TFTDisplay( TFT_eSPI& _tft ) : m_tft( _tft ) {
		m_backColor = 0;
		m_tft.setSwapBytes( false );
	}
//	TFTDisplay( Adafruit_ST7789& _tft ) : m_tft( _tft ) {
//		m_backColor = 0;
//	}

	void	Clear() { Clear( m_backColor ); }
	void	Clear( U16 RGB );
	void	Clear( U8 R, U8 G, U8 B ) { Clear( RGB16( R, G, B ) ); }

	void	DrawBitmap( const BMP& _bitmap, S16 _x, S16 _y );

	void	SetTextProperties( U8 _size, U16 _cursorX, U16 _cursorY, U16 _color );

	void	print( const char* _text );
	void	println( const char* _text );
	void	printf( const char* _text, ... );

	void 	PrintTest();
	void 	PrintTest2();

	// RGB 24 → 16 bits (big endian, TFT-ready 16-bits color)
	static U16	RGB16_BigEndian( U8 R, U8 G, U8 B ) { return SwapBytes( RGB16( R, G, B ) ); }

	// TFT display expects big-endian bytes when dealing with raw pixels!!!!
	static U16	SwapBytes( U16 _bytes ) { return ((_bytes & 0xFF) << 8) | (_bytes >> 8); }
};