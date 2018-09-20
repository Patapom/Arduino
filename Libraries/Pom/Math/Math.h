//
float	log2( float v );	// = ln(x)/ln(2)

S32		clamp( S32 v, S32 _min, S32 _max );
float	clamp( float v, float _min, float _max );

// Swaps between big and little endian
void	SwapBytes( U16& _value );
void	SwapBytes( U32& _value );
void	SwapBytes( U64& _value );
