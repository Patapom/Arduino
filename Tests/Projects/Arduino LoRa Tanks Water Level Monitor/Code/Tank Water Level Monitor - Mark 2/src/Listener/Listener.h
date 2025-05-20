////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LISTENER SIDE
//  • The listener side is located on the local PC, it waits for the monitors to broadcast some messages
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "../Global.h"

class Listener {
	private:	// TYPES

		// Before refactor (8 bytes structure)
//		static const U32	MEASUREMENTS_COUNT = 64;				// = 512 bytes (1603 bytes, 78.3% of RAM)
//		static const U32	MEASUREMENTS_COUNT = 128;				// = 1024 bytes (2115 bytes, 103.3% of RAM)
//		static const U32	MEASUREMENTS_COUNT = 110;				// = 880 bytes (1971 bytes, 96.2% of RAM)

		// After refactor (4 bytes structure)
static const U32	MEASUREMENTS_COUNT = 8;
//		static const U32	MEASUREMENTS_COUNT = 64;				// = 256 bytes (1329 bytes, 64.9% of RAM)
//		static const U32	MEASUREMENTS_COUNT = 128;				// = 512 bytes (1585 bytes, 77.4% of RAM)
//		static const U32	MEASUREMENTS_COUNT = 160;				// = 640 bytes (1713 bytes, 83.6% of RAM)
		// BREAKS BEYOND THAT!! LORA FAILS AFTER TIMEOUT!
//		static const U32	MEASUREMENTS_COUNT = 192;				// = 768 bytes (1841 bytes, 89.9% of RAM)		END LIMIT AT 89.7% (211 free bytes)
//		static const U32	MEASUREMENTS_COUNT = 159;				// = 954 bytes (2041 bytes, 99.7% of RAM) <= Can't use that as it prevents proper execution since some functions need a lot of stack memory

		static const S32	MEASURE_DELTA_TIME_TOLERANCE_S = 10;	// Allow up to 10s of tolerance between measurements to consider them as identical

	public: static const int	PIN_BUTTON = 6;

	public:	// FIELDS

		Time_ms		m_loopTime;			// The time at which the last call to loop() occurred (used to keep track of real time)
		Time_ms		m_lastUpdateTime;	// The time at which we last checked the serials for an update

		// Array of measurements
		U32			m_measurementsCount;
		Measurement	m_measurements[MEASUREMENTS_COUNT];
		S32			m_lastMeasurementTime_s;	// The time (in seconds) when we received the last measurement in the array, relative to the device's start time
												// (all other measurement times can be retrieved by following the trail of delta times)

	public:
		Listener() : m_measurementsCount( 0 ), m_lastMeasurementTime_s( -100000L ) {
		}

		void	setup();
		void 	loop();

	private:
		U32		RegisterMeasurements( const Time_ms& _now, Measurement* _newMeasurements, U32 _newMeasurementsCount );
		void	SendMeasurements();
};
