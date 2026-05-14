#include "IconsPalette.h"

U32	IconsPalette::Append( fs::File& _file ) {
	U32		index = m_icons.size();
	if ( index >= m_icons.capacity() )
		throw "Palette full! Reserve more capacity!";

	m_icons.emplace_back();
	BMP&	img = m_icons[index];
	if ( !img.Open24( _file ) )
		throw "Failed to open BMP file!";

	return index;
}

U32	IconsPalette::Append( fs::FS& _fileSystem, const char* _fileName ) {
	U32		index = m_icons.size();
	if ( index >= m_icons.capacity() )
		throw "Palette full! Reserve more capacity!";

	m_icons.emplace_back();
	BMP&	img = m_icons[index];
	if ( !img.Open24( _fileSystem, _fileName ) )
		throw "Failed to open BMP file!";

	return index;
}

void	IconsPalette::DrawBitmaps( TFTDisplay& _display, U8 _startIndex, U8 _count, S16 X, S16 Y ) const {
	if ( _startIndex + _count > m_icons.size() ) {
Serial.println( "Bitmap indices out of range!" );
		throw "Bitmap indices out of range!";
	}

	for ( U8 count=0; count < _count; count++ ) {
		const BMP&	bmp = m_icons[_startIndex + count];
		_display.DrawBitmap( bmp, X, Y );
		X += bmp.m_width;
	}
}
