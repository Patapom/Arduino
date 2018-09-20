
//////////////////////////////////////////////////////////////////////////
// Serial printf from https://arduino.stackexchange.com/users/65/asheeshr
// https://arduino.stackexchange.com/questions/176/how-do-i-print-multiple-variables-in-a-string
//
// Fixed and upgraded by me for consistency and '\n' handling
//
#ifndef ARDBUFFER
#define ARDBUFFER 16
#endif

U32	SerialPrintf( char *str, ... );

//////////////////////////////////////////////////////////////////////////
// A digital read that returns true only if the pin has the expected value for the required duration (user must provide the counter)
// Example:
//	bool	sw = bufferedRead( ROT_PIN_SWITCH, HIGH, 8, switchCounter );	// Will return true if the PIN returns HIGH for more than 8 calls
//
bool	BufferedRead( U8 _pin, int _expectedValue, U8 _duration, U8& _counter );