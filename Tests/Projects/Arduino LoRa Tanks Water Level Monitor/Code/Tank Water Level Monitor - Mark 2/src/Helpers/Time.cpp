#include "Global.h"

U32	Time_ms::ms_lastMillis = 0;
U32 Time_ms::ms_timeOverflowCounter = 0;

void	Time_ms::GetTime() {
	time_ms = millis();
	if ( time_ms < ms_lastMillis ) {
		ms_timeOverflowCounter++;  // One more overflow!
	}
	ms_lastMillis = U32(time_ms);
  	*((U32*) time_ms) = ms_timeOverflowCounter;	// Store as MSW
}
