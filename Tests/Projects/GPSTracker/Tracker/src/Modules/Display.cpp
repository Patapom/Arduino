#include "Display.h"

void	TFTDisplay::Clear( U16 RGB ) {
	m_backColor = RGB;
//	m_tft.fillScreen( ST77XX_BLACK );
//	m_tft.fillScreen( ST77XX_RED );
	m_tft.fillScreen( RGB );
}

void	TFTDisplay::SetTextProperties( U8 _size, U16 _cursorX, U16 _cursorY, U16 _color ) {
	m_tft.setTextWrap( false );
//	m_tft.fillScreen(ST77XX_BLACK);
	m_tft.setTextSize( _size );
	m_tft.setCursor( _cursorX, _cursorY );
	m_tft.setTextColor( _color );
}

void	TFTDisplay::print( const char* _text ) {
	// Check text is not out of the screen
	// If it is, then wrap the cursor back to the top...
	S16	x, y;
	U16	w, h;
	m_tft.getTextBounds( _text, m_tft.getCursorX(), m_tft.getCursorY(), &x, &y, &w, &h );
	if ( y > m_tft.height() ) {
		// Clear screen and wrap around...
		Clear();
		m_tft.setCursor( m_tft.getCursorX(), 0 );
	}

	m_tft.print( _text );

Serial.print( _text );
}

void	TFTDisplay::println( const char* _text ) {
	char	temp[256];
	strncpy( temp, _text, 256-3 );
	char*	end = temp + strlen(temp);
	*end++ = '\r';
	*end++ = '\n';
	*end++ = '\0';

	print( temp );
}

void	TFTDisplay::printf( const char* _text, ... ) {
	va_list	arg;
	va_list	copy;
	va_start( arg, _text );
	va_copy( copy, arg );

	char	temp[256];
	int		len = vsnprintf( temp, sizeof(temp), (const char*) _text, copy );

	va_end( copy );

	print( temp );
}


float p = 3.1415926;
 
void TFTDisplay::PrintTest() {
	Adafruit_ST7789&	tft = m_tft;

	tft.setTextWrap(false);
	tft.fillScreen(ST77XX_BLACK);
	tft.setCursor(0, 30);
	tft.setTextColor(ST77XX_RED);
	tft.setTextSize(1);
	tft.println("Hello World!");
	tft.setTextColor(ST77XX_YELLOW);
	tft.setTextSize(2);
	tft.println("Hello World!");
	tft.setTextColor(ST77XX_GREEN);
	tft.setTextSize(3);
	tft.println("Hello World!");
	tft.setTextColor(ST77XX_BLUE);
	tft.setTextSize(4);
	tft.print(1234.567);
}

void TFTDisplay::PrintTest2() {
	Adafruit_ST7789&	tft = m_tft;

	tft.setCursor(0, 0);
	tft.fillScreen(ST77XX_BLACK);
	tft.setTextColor(ST77XX_WHITE);
	tft.setTextSize(0);
	tft.println("Hello World!");
	tft.setTextSize(1);
	tft.setTextColor(ST77XX_GREEN);
	tft.print(p, 6);
	tft.println(" Want pi?");
	tft.println(" ");
	tft.print(8675309, HEX); // print 8,675,309 out in HEX!
	tft.println(" Print HEX!");
	tft.println(" ");
	tft.setTextColor(ST77XX_WHITE);
	tft.println("Sketch has been");
	tft.println("running for: ");
	tft.setTextColor(ST77XX_MAGENTA);
	tft.print(millis() / 1000);
	tft.setTextColor(ST77XX_WHITE);
	tft.print(" seconds.");
}
