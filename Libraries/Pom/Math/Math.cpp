#include "..\Pom.h"

float	log2( float v ) { return 1.4426950408889634073599246810019f * log( v ); }	// = ln(x)/ln(2)

float	lerp( float a, float b, float t ) {
	return a + (b-a) * t;
}
float	smoothstep( float a, float b, float t ) {
	b -= a;
	return a + t * t * (3.0f - 2.0f * t) * b;
}

S32		clamp( S32 v, S32 _min, S32 _max )			{ return max( _min, min( _max, v ) ); }
float	clamp( float v, float _min, float _max )	{ return max( _min, min( _max, v ) ); }

// Swaps between big and little endian
// Code from https://codereview.stackexchange.com/questions/64797/byte-swapping-functions
void	SwapBytes( U16& a ) {
	a = ((a & 0x00FF) << 8) | ((a & 0xFF00) >> 8);
}
void	SwapBytes( U32& a ) {
	a = ((a & 0x000000FF) << 24) |
		((a & 0x0000FF00) <<  8) |
		((a & 0x00FF0000) >>  8) |
		((a & 0xFF000000) >> 24);
}
void	SwapBytes( U64& a ) {
	a = ((a & 0x00000000000000FFULL) << 56) | 
		((a & 0x000000000000FF00ULL) << 40) | 
		((a & 0x0000000000FF0000ULL) << 24) | 
		((a & 0x00000000FF000000ULL) <<  8) | 
		((a & 0x000000FF00000000ULL) >>  8) | 
		((a & 0x0000FF0000000000ULL) >> 24) | 
		((a & 0x00FF000000000000ULL) >> 40) | 
		((a & 0xFF00000000000000ULL) >> 56);
}
