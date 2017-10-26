#include "Arduino.h"
#include "Math.h"

float	log2( float v ) { return 1.4426950408889634073599246810019f * log( v ); }	// = ln(x)/ln(2)

int		clamp( int v, int _min, int _max )			{ return max( _min, min( _max, v ) ); }
float	clamp( float v, float _min, float _max )	{ return max( _min, min( _max, v ) ); }
