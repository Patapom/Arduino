#pragma once

#include "../Global.h"
#include "BMPFile.h"
#include "../Modules/Display.h"

class IconsPalette {
public:
	std::vector<BMP>	m_icons;

public:
	IconsPalette( U32 _capacity ) { m_icons.reserve( _capacity ); }
	~IconsPalette() {}

	// Appends an icon, returns the index of the icon
//	U32	Append( const BMP& _BMP );
	U32	Append( fs::File& _file );
	U32	Append( fs::FS& _fileSystem, const char* _fileName );

	BMP&	operator[]( U8 _index ) { return m_icons[_index]; }

	void	DrawBitmaps( TFTDisplay& _display, U8 _startIndex, U8 _count, S16 X, S16 Y ) const;
};