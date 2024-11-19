////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Measures the distance in meters using the HC-SR04
// According to the documentation, we have to send a 10µs pulse to the trigger pin to send an echo,
//  then depending on the time before we receive an echo, we can measure the distance
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void	SetupPins_HCSR04( U8 _pinTrigger, U8 _pinEcho );

// Returns the raw time in µs before we receive an echo (this is the total round trip back and forth Time Of Flight), or 0 if the pulse timed out
U32 	MeasureEchoTime( U8 _pinTrigger, U8 _pinEcho );

float	ConvertTimeOfFlightToDistance( U32 _timeOfFlight_microseconds, float _speedOfSound_metersPerSecond );
float	ConvertTimeOfFlightToDistance( U32 _timeOfFlight_microseconds );

float	MeasureDistance( U8 _pinTrigger, U8 _pinEcho );
