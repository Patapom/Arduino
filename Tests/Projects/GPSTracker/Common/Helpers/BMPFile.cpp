#include "BMPFile.h"
//#include <LittleFS.h>

BMP::~BMP() {
	delete[] m_bitmap;
}

bool	BMP::Open24( const char* _fileName ) {
	if ( !m_fileSystem.exists( _fileName ) )
		return false;

	File	file = m_fileSystem.open( _fileName );

	// Ensure we're reading the right format
	if ( read16( file ) != 0x4D42 ) {
		Serial.println( "BMP format not recognized." );
		file.close();
		return false;
	}

	read32( file );
	read32( file );
	U32	seekOffset = read32( file );	// Offset to actual bitmap data
	read32( file );
	m_width = read32( file );
	m_height = read32( file );

	if ( (read16( file ) != 1) || (read16( file ) != 24) || (read32( file ) != 0) ) {
		Serial.println( "Only 24-bits BMPs are supported..." );
		file.close();
		return false;
	}

//	y += h - 1;
//
//	bool oldSwapBytes = tft.getSwapBytes();
//	tft.setSwapBytes(true);

	file.seek( seekOffset );

	U16	padding = (4 - ((m_width * 3) & 3)) & 3;
	U16	tempScanlineSize = m_width * 3 + padding;
	U8	tempScanline[tempScanlineSize];

//	m_scanlineSize = (m_width * 3 + 3) & ~3;
	m_bitmap = new U16[m_height * m_width];	// Final format is R5G6B5

	U16*	scanline = m_bitmap + m_width * (m_height-1);	// Reverse order because BMP files are stored in reversed order!
	for ( U16 Y=0; Y < m_height; Y++, scanline-=m_width ) {
		file.read( tempScanline, tempScanlineSize );
		U8*  sourcePtr = tempScanline;
		U16* targetPtr = scanline;

		// Convert 24 to 16-bit colours
		for ( U16 X=0; X < m_width; X++ ) {
			U8	b = *sourcePtr++;
			U8	g = *sourcePtr++;
			U8	r = *sourcePtr++;

			*targetPtr++ = SwapBytes( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) );	// RGB format
//			*targetPtr++ = SwapBytes( ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3) );	// BGR format
		}
	}

	file.close();

	return true;
}

void	BMP::CreateTest( U16 _w, U16 _h ) {
	m_width = _w;
	m_height = _h;
	m_bitmap = new U16[m_width * m_height];

	U16*	scanline = m_bitmap;
	U32		RGB;
	for ( U32 Y=0; Y < m_height; Y++, scanline+=m_width ) {
		U16*	ptr = scanline;
		for ( U32 X=0; X < m_width; X++ ) {
			*ptr++ = SwapBytes( ((RGB >> 8) & 0xF800) | ((RGB >> 5) & 0x7E0) | ((RGB >> 3) & 0x1F) );	// RGB format
//			*ptr++ = SwapBytes( (RGB >> 19) | ((RGB >> 5) & 0x7E0) | ((RGB << 8) & 0xF800) );			// BGR format

// Horizontal gradients for testing purpose
//*ptr++ = SwapBytes( (X & 0xF8) >> 3 );	// Blue
//*ptr++ = SwapBytes( (X & 0xFC) << 3 );	// Green
//*ptr++ = SwapBytes( (X & 0xF8) << 8 );	// Red

			RGB = ((X << 17) & 0xFF0000)	// Red gradient on X (looping every 128 pixels)
				| ((Y << 9) & 0x00FF00)		// Green gradient on Y (looping every 128 pixels)
				| ((0) & 0x0000FF);			// Blue gradient inviible
		}
	}
}


// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.
U16 BMP::read16( File& f ) {
	U16 result;
	((U8*) &result)[0] = f.read(); // LSB
	((U8*) &result)[1] = f.read(); // MSB
	return result;
}

U32	BMP::read32( File& f ) {
	U32 result;
	((U8*) &result)[0] = f.read(); // LSB
	((U8*) &result)[1] = f.read();
	((U8*) &result)[2] = f.read();
	((U8*) &result)[3] = f.read(); // MSB
	return result;
}

/*
// Bodmer's BMP image rendering function
void drawBmp( const char* filename, U16 x, U16 y ) {

	if ( (x >= tft.width()) || (y >= tft.height()) )
		return;

	fs::File bmpFS;

	// Open requested file on SD card
	bmpFS = SPIFFS.open(filename, "r");

	if (!bmpFS)
	{
	Serial.print("File not found");
	return;
	}

	U32 seekOffset;
	U16 w, h, row, col;
	U8  r, g, b;

	U32 startTime = millis();

	if (read16(bmpFS) == 0x4D42)
	{
	read32(bmpFS);
	read32(bmpFS);
	seekOffset = read32(bmpFS);
	read32(bmpFS);
	w = read32(bmpFS);
	h = read32(bmpFS);

	if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
	{
		y += h - 1;

		bool oldSwapBytes = tft.getSwapBytes();
		tft.setSwapBytes(true);
		bmpFS.seek(seekOffset);

		U16 padding = (4 - ((w * 3) & 3)) & 3;
		U8 lineBuffer[w * 3 + padding];

		for (row = 0; row < h; row++) {
		
		bmpFS.read(lineBuffer, sizeof(lineBuffer));
		U8*  bptr = lineBuffer;
		U16* tptr = (U16*)lineBuffer;
		// Convert 24 to 16-bit colours
		for (U16 col = 0; col < w; col++)
		{
			b = *bptr++;
			g = *bptr++;
			r = *bptr++;
			*tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
		}

		// Push the pixel row to screen, pushImage will crop the line if needed
		// y is decremented as the BMP image is drawn bottom up
		tft.pushImage(x, y--, w, 1, (U16*)lineBuffer);
		}
		tft.setSwapBytes(oldSwapBytes);
		Serial.print("Loaded in "); Serial.print(millis() - startTime);
		Serial.println(" ms");
	}
	else Serial.println("BMP format not recognized.");
	}
	bmpFS.close();
}
*/