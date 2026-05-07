#pragma once

#include "../Global.h"
#include <FS.h>

class File;

class BMP {
	friend class Display;
public:
	fs::FS&	m_fileSystem;

	U16		m_width, m_height;
	U16		m_scanlineSize;
	U16*	m_bitmap;			// Pixel format is RRRRR GGGGGG BBBBB *BIG ENDIAN* (i.e. pre-swapped bytes, ready for the TFT screen)

public:
	BMP( fs::FS& _fileSystem ) : m_fileSystem( _fileSystem ), m_bitmap( nullptr ) {}
	~BMP();

	// Opens a 24-bits BMP file
	bool	Open24( const char* _fileName );

	void	CreateTest( U16 _w, U16 _h );

private:
	U16	read16( fs::File& _file );
	U32	read32( fs::File& _file );

	// TFT display expects big-endian bytes!!!!
	U16	SwapBytes( U16 _bytes ) { return ((_bytes & 0xFF) << 8) | (_bytes >> 8); }
};