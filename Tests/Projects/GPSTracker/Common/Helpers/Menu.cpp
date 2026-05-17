#include "Menu.h"

U16	Menu::DrawString( const char* _string, U16 X, U16 Y, U16 _W, U16 _H, bool _selected ) {

	U16	textWidth = m_display.m_tft.textWidth( _string );
	U16	textHeight = m_display.m_tft.fontHeight();

	X += m_X;
	Y += m_Y;
//Serial.printf( "Origin = %d, %d\r\n", X, Y );

	U16	W = _W == -1 ? m_textMarginX + textWidth + m_textMarginX : _W;
	U16	H = _H == -1 ? m_textMarginY + textHeight + m_textMarginY : _H;

	// Fill background
	if ( _selected ) {
		// Fill selection
		int	radius = max( m_textMarginX, m_textMarginY );
		m_display.m_tft.fillRoundRect( X, Y, W, H, radius, m_selectionBackColor );
		m_display.m_tft.drawRoundRect( X, Y, W, H, radius, m_selectionForeColor );
	} else {
		m_display.m_tft.fillRect( X, Y, W, H, m_backColor );
	}

	// Draw string
// TODO: use text datum
	S16	textX = X + ((W - textWidth) >> 1);
	S16	textY = Y + ((H - textHeight) >> 1);
	m_display.m_tft.setCursor( textX, textY );
	m_display.m_tft.setTextColor( m_foreColor, _selected ? m_selectionBackColor : m_backColor );
	m_display.m_tft.drawString( _string, textX, textY );

	return H;
}
