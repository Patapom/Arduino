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

		// After refactor (6 bytes structure)
		static const U32	MEASUREMENTS_COUNT = 64;				// = 384 bytes (1477 bytes, 72.1% of RAM)
//		static const U32	MEASUREMENTS_COUNT = 110;				// = 660 bytes (1747 bytes, 85.3% of RAM)
//		static const U32	MEASUREMENTS_COUNT = 124;				// = 744 bytes (1837 bytes, 89.7% of RAM => 211 free bytes)
		// BREAKS BEYOND THAT!! LORA FAILS AFTER TIMEOUT!
//		static const U32	MEASUREMENTS_COUNT = 128;				// = 768 bytes (1861 bytes, 90.9% of RAM)
//		static const U32	MEASUREMENTS_COUNT = 159;				// = 954 bytes (2041 bytes, 99.7% of RAM) <= Can't use that as it prevents proper execution since some functions need a lot of stack memory

		static const U64	MEASURE_DELTA_TIME_TOLERANCE_S = 10;	// Allow up to 10s of tolerance between measurements to consider them as identical

		// The local measurement structure (size = 6 bytes)
		struct LocalMeasurement {
			S32			time_s;				// The time of the measurement (in seconds), relative to the start time of the listener device
			U16			rawValue_micros;	// The raw time of flight from the sensor, in µseconds
		};

	public: static const int	PIN_BUTTON = 6;

	public:	// FIELDS

		DateTime	m_globalTime;		// The global clock time, sent by the PC through serial
		Time_ms		m_clockSetTime;		// The time when the clock was set

		Time_ms		m_loopTime;			// The time at which the last call to loop() occurred (used to keep track of real time)
		Time_ms		m_lastUpdateTime;	// The time at which we last checked the serials for an update

		// Array of measurements
		U32					m_measurementsCount;
		LocalMeasurement	m_measurements[MEASUREMENTS_COUNT];

	public:
		Listener() : m_measurementsCount( 0 ) {
		}

		void	setup();
		void 	loop();

	private:
		void	ReadCommand();
		bool	ReadDateTime( char* _dateTime, U32 _dateTimeLength );
		U32		RegisterMeasurements( LocalMeasurement* _measurements, U32 _measurementsCount );
		void	SendMeasurements();
		S32		ConvertLocal2GlobalTime( S32 _localTime_ms );
};
