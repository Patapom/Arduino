#include "MenuMain.h"

void	MenuMain::MoveUp() { m_selection = max( 0, m_selection-1 ); }
void	MenuMain::MoveDown() { m_selection = min( 3, m_selection+1 ); }
void	MenuMain::MoveLeft() {}
void	MenuMain::MoveRight() {}

void	MenuMain::PaintBackground() {
	m_display.m_tft.fillRect( m_X, m_Y, m_width, m_height, TFT_RED );
}

void	MenuMain::Paint() {
	S16	X = 20;
	S16	Y = 20;
	U16	W = 100;
	U16	H = 20;

	U16	h = DrawString( "MENU TEST 1", X, Y, W, H, m_selection == 0 );	Y += 2 * h;
		h = DrawString( "MENU TEST 2", X, Y, W, H, m_selection == 1 );	Y += 2 * h;
		h = DrawString( "MENU TEST 3", X, Y, W, H, m_selection == 2 );	Y += 2 * h;
		h = DrawString( "MENU TEST 4", X, Y, W, H, m_selection == 3 );	Y += 2 * h;
}
