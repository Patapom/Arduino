#pragma once

#include <vector>

typedef unsigned int	uint;
typedef unsigned short	ushort;
typedef unsigned char	byte;
typedef unsigned long	U32;
typedef unsigned short	U16;
typedef unsigned char	U8;
typedef signed long		S32;
typedef signed short	S16;
typedef signed char		S8;

// Home latitude
constexpr double	homeLatitude = 49.516223500;
constexpr double	homeLongitude = -124.362310167;
constexpr double	homeAltitude_m = 60.0;

//RawDegrees	homeLatitude { 49, 516223500, false };	// Home Latitude 49.516223500
//RawDegrees	homeLongitude { 124, 362310167, true };	// Home Longitude -124.362310167
// Minimum latitude is the bottom-left corner of Lasqueti so we can only get positive values from the delta location
//RawDegrees	minLatitude {   49, 516100000, false };
//RawDegrees	minLongitude { 124, 362500000, true };

// RGB24 → RGB16
#define RGB16( R, G, B )  U16( (B & 0x1F) | ((G & 0xFC) << 3) | ((R & 0xF8) << 8) )
