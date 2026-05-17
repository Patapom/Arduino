#pragma once

#include "../Global.h"
#include <FS.h>

class BMP {
	friend class Display;
public:

	U16		m_width, m_height;
	U16		m_scanlineSize;
	U16*	m_bitmap;			// Pixel format is RRRRR GGGGGG BBBBB *BIG ENDIAN* (i.e. pre-swapped bytes, ready for the TFT screen)

public:
	BMP() : m_bitmap( nullptr ) {}
	~BMP();

	// Opens a 24-bits BMP file
	bool	Open24( fs::FS&	_fileSystem, const char* _fileName );
	bool	Open24( fs::File& _file );

	void	CreateTest( U16 _w, U16 _h );

private:
	U16		Read16( fs::File& _file );
	U32		Read32( fs::File& _file );
};