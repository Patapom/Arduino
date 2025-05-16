////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LISTENER SIDE
//  • The listener side is located on the local PC, it waits for the monitors to broadcast some messages
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "../Global.h"

//#define PIN_BUTTON	6

class Listener {
	private:	// TYPES

//		static const U32	MEASUREMENTS_COUNT = 64;						// = 512 bytes (1603 bytes, 78.3% of RAM)
//		static const U32	MEASUREMENTS_COUNT = 128;						// = 1024 bytes (2115 bytes, 103.3% of RAM)
		static const U32	MEASUREMENTS_COUNT = 110;						// = 880 bytes (1971 bytes, 96.2% of RAM)
		static const U64	MEASURE_DELTA_TIME_TOLERANCE_MS = 1000 * 10;	// Allow up to 10s of tolerance between measurements to consider them as identical

		// The local measurement structure (size = 8 bytes)
		struct LocalMeasurement {
			U64			time_ms : 48;		// The time of the measurement, relative to the start time of the listener device
			U16			rawValue_micros;	// The raw time of flight from the sensor, in µseconds
		};

	public:	// FIELDS

		DateTime	m_globalTime;	// The global time, send by the PC through serial
		Time_ms		m_clockSetTime;	// The time when the clock was set
		Time_ms		m_startTime;	// The time when this board came online and started listening to the monitors
		Time_ms		m_loopTime;		// The time at which the last call to loop() occurred (used to keep track of real time)

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
		U64		ConvertLocal2GlobalTime( U64 _localTime_ms );
	};
