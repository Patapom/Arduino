#pragma once

#include "../Global.h"
#include "Display.h"
#include "IconsPalette.h"

// The Menu class is an abstract class offering the opportunity to render and navigate through a single Menu panel
class Menu {
protected:
	TFTDisplay&			m_display;
	const IconsPalette&	m_icons;

public:

	// Rendering window
	U16	m_X = 0, m_Y = 0;
	U16	m_width = 100, m_height = 100;

	// Color scheme
	U16	m_backColor = RGB16( 0, 0, 0 );
	U16	m_foreColor = RGB16( 0xFF, 0xFF, 0xFF );
	U16	m_selectionBackColor = RGB16( 0x30, 0x20, 0 );
	U16	m_selectionForeColor = RGB16( 0xFF, 0, 0 );

	// Text properties
	U8	m_textMarginX = 4;
	U8	m_textMarginY = 4;

public:
	Menu( TFTDisplay& _display, const IconsPalette& _icons ) : m_display( _display ), m_icons( _icons ) {}
	virtual ~Menu() {}

	// Sets the dimensions of the rendering window
	void			SetWindow( U16 X, U16 Y, U16 _width, U16 _height ) { m_X = X; m_Y = Y; m_width = _width; m_height = _height; }

	// Sets the color scheme
	void			SetColors( U16 _backColor, U16 _foreColor, U16 _selectionBackColor, U16 _selectionForeColor ) {
		m_backColor = _backColor;
		m_foreColor = _foreColor;
		m_selectionBackColor = _selectionBackColor;
		m_selectionForeColor = _selectionForeColor;
	}

	// Sets the text properties
	void			SetTextAttributes( U8 _size, U8 _marginX=4, U8 _marginY=4, U8 _datum=CC_DATUM ) {
		m_display.m_tft.setTextSize( _size );
		m_display.m_tft.setTextDatum( _datum );
		m_textMarginX = _marginX;
		m_textMarginY = _marginY;
	}

	// Navigation
	virtual void	MoveUp() = 0;
	virtual void	MoveDown() = 0;
	virtual void	MoveLeft() = 0;
	virtual void	MoveRight() = 0;

	// Rendering
	virtual void	PaintBackground() = 0;
	virtual void	Paint() = 0;

protected:	// Helpers
			// NOTE: All drawing functions take (X,Y) relative to the window's top-left corner

	// Draws a string + background rectangle + optional selection frame
	// Returns the height of the rendered text rectangle
	U16	DrawString( const char* _string, U16 X, U16 Y, U16 _W=-1, U16 _H=-1, bool _selected=false );

};
