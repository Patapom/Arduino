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
	U16*	m_bitmap;

public:
	BMP( fs::FS& _fileSystem ) : m_fileSystem( _fileSystem ), m_bitmap( nullptr ) {}
	~BMP();

	// Opens a 24-bits BMP file
	bool	Open24( const char* _fileName );

private:
	U16	read16( fs::File& _file );
	U32	read32( fs::File& _file );

};