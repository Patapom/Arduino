#include "Global.h"

void	Time_ms::GetTime() {
	U32*	time = (U32*) &time_ms;

	// Only deal with LSW
	U32	oldMillis = time[0];
	time[0] = millis();

	// Handle possible overflow
	if ( time[0] < oldMillis ) {
		time[1]++;
	}
}
