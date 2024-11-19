////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LISTENER SIDE
//  • The listener side is located on the local PC, it waits for the monitors to broadcast some messages
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "../Global.h"

#define PIN_BUTTON	6

static const int  SERVER_WAIT_INTERVAL_MS = 11000; // Wait 11 seconds before retrying (must always be > to client poll interval to give the client enough time to respond)

class Listener {

//	Time_ms	m_startTime;	// Time at which the loop starts

public:
	void	setup();
	void 	loop();

};
