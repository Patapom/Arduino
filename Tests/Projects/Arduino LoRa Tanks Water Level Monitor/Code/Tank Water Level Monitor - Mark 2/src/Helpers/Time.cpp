#include "Global.h"

U32	lastMillis = 0;
U32 timeOverflowCounter = 0;

void	Time_ms::GetTime() {
	time0_ms = millis();
	if ( time0_ms < lastMillis ) {
		timeOverflowCounter++;  // One more overflow!
	}
	lastMillis = time0_ms;
  	time1_ms = timeOverflowCounter;
}
