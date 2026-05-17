#pragma once

#include <Menu.h>

// The main menu class offering the choice in basic tasks & settings
class MenuMain : public Menu {
public:

	U8		m_selection = 0;

public:
	MenuMain( TFTDisplay& _display, const IconsPalette& _icons ) : Menu( _display, _icons ) {}
	virtual ~MenuMain() {}

	// Navigation
	virtual void	MoveUp();
	virtual void	MoveDown();
	virtual void	MoveLeft();
	virtual void	MoveRight();

	// Rendering
	virtual void	PaintBackground();
	virtual void	Paint();
};
