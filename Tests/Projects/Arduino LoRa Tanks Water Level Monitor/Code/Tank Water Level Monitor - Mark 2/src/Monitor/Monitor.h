////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MONITOR SIDE
//  • The monitor side executes a measurement every N seconds, sends the result to the listener (PC), then goes back to deep sleep
//    ☼ If the rate of change in the measured value increases/decreases then the sleeping time is reduced so measurements are taken more often
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "../Global.h"

#define DEBUG_MONITOR

//#define USE_LOW_POWER_IDLE		// Define this to use the LowPower library and enter idle mode for 8s instead of a delay (delay still eats energy)
#ifdef USE_LOW_POWER_IDLE
#include <LowPower.h>
#endif

class Monitor {

	static constexpr U32	LORA_ADDRESS = 1;		// Be careful not to be the same as monitor (i.e. channel 0)!

	static constexpr float	FULL_TANK_VOLUME_L = 4000.0f;	// Full tank volume (litres)
	static constexpr float	FULL_TANK_DISTANCE_M = 1.73f;	// Tank height when full (meters)

	#if defined(DEBUG_MONITOR)
		static constexpr U32	MAX_SLEEP_DURATION_S = 16;
		static constexpr U32	MIN_SLEEP_DURATION_S = 8;
	#else
		static constexpr U32	MAX_SLEEP_DURATION_S = 10 * 60;	// Sleep for 10 minutes when nothing is happening (seconds)
		static constexpr U32	MIN_SLEEP_DURATION_S = 30;		// Sleep for 30 seconds when flow is high (seconds)
	#endif

	static constexpr float	MIN_FLOW_RATE_LPM = 0.1f;		// Slow flow => 0.1 litres per minute = 6 litres per hour (= no flow with our small leak)
	static constexpr float	MAX_FLOW_RATE_LPM = 2.0f;		// Fast flow => 2 litres per minute is considered a high flow rate (typical shower)

	// HC-SR04 Ultrasound Distance Measurement device
	static constexpr U32	PIN_HCSR04_TRIGGER = 6;
	static constexpr U32	PIN_HCSR04_ECHO = 7;

	Measurement	m_measurements[16];	// The circular array of measurements
	U32			m_measurementIndex;

	U32			m_totalSleepTime_s;

public:
	Monitor() {
		m_measurementIndex = 0;
		memset( m_measurements, 0, 16 * sizeof(Measurement) );
		m_totalSleepTime_s = 0;
	}

	void setup();
	void loop();

private:

	Measurement&	MeasureDistance();

};