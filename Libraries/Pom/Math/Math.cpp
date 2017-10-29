#include "..\Pom.h"

float	log2( float v ) { return 1.4426950408889634073599246810019f * log( v ); }	// = ln(x)/ln(2)

S32		clamp( S32 v, S32 _min, S32 _max )			{ return max( _min, min( _max, v ) ); }
float	clamp( float v, float _min, float _max )	{ return max( _min, min( _max, v ) ); }
