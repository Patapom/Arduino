#pragma once

#include "../Global.h"

#include <TinyGPSPlus.h>

// U-BLOX Neo-6M documentation:	u-blox-F10-SPG-6.00_InterfaceDescription_UBX-23002975.pdf
//
// TODO:
//	• Use "UBX-MGA-INI-POS_LLH" to specify initial lat/lon/elevation
//		→ Use "UBX-INF-TEST" ? (although it's marked as "output" only ??? How do we make the module spit a test sequence?)

class	GPS {
	// Source for EMA: https://www.investopedia.com/ask/answers/difference-between-simple-exponential-moving-average/
	static constexpr double	EXPONENTIAL_MOVING_AVERAGE_TIME_PERIOD = 10;
	static constexpr double	EXPONENTIAL_MOVING_AVERAGE_FACTOR = 2 / (1 + EXPONENTIAL_MOVING_AVERAGE_TIME_PERIOD);	// = 2 / (1 + "Time Period")

public:
	class 	DateTime {
	public:
		U16	Y;
		U8	M;
		U8	D;
		U8	h;
		U8	m;
		U8	s;

		// Converts a UTC time into a local PST time
		void	ToLocal( DateTime& _localTime );
	};

	enum class FIX_STATUS {
		// Fix/No fix
		NO_FIX = 0,					// No satellite fix
		FOUND_FIX,					// Successfully found a fix (read HDOP, satellites count and Quality field for overall quality)

		// Errors
		ERROR_NO_GPS_MODULE = -1,	// Returns failure after 5s without any life sign from the 
		ERROR_TIME_OUT = -2,		// The search for a fix timed out without success
		SEARCH_ABORTED_BY_USER = -3,// User callback returned false to abort search
	};

	// A copy of the TinyGPS version with added information
	enum Quality {
		Invalid = '0',		// No fix, invalid location
		STD_GPS = '1',		// Standard GPS fix, using 4 satellites (minimum)
		DIFF_GPS = '2',		// Differential GPS fix, additional GPS corrections using ground stations (more precise, ~1-3m)
		PPS = '3',			// Position from very precise time signals (rarely used, in specialized systems)
		RTK = '4',			// Real-Time Kinematics, very high precision (cm level, drones & topography)
		FloatRTK = '5',		// Floating precision RTK, precision to the decimeter
		Estimated = '6',	// Estimated position (dead reckoning using inertial/interpolation, no GPS available)
		Manual = '7',		// Manual position
		Simulated = '8'		// Simulated position (testing)
	};

	struct Data {
		FIX_STATUS	fixStatus = FIX_STATUS::NO_FIX;
		U8			satellitesCount = 0;
		float 		HDOP = 10;	// Horizontal Dilution of Precision.
								// Typical values are:
								//		< 1		Excellent	~1–3 m
								//		1 – 2	Very good	~3–5 m
								//		2 – 5	OK			~5–15 m
								//		> 5		Bad 		> 20 m
								//		> 10	Very bad	inutilisable		

		// Location
		U32			lastValidLocation_Time_ms = -1;

		Quality		locationQuality = Quality::Invalid;

		// We're storing latitudes and longitudes as 64-bits double precision which should be enough to account
		//	for the precision of the raw measures sent by the module which can go up to 1e-9 units
		// If we're assuming degree values between -180 and +180 degrees and 9 digits of precision, we need to store a number with an magnitude of 1e12 = 39.86 bits of precision
		// We know 64-bits floats use 53 bits of mantissa and 11 bits of exponent, which is largely enough to accommodate our needs here...
		//
		double		latitude;
		double		longitude;

		// These values are accumulated using an exponential moving average (EMA) with a common smoothing length of 10 values when accuracy (HDOP) is low or all the way down to 2 values when accuracy is high
		double		avgLatitude;
		double		avgLongitude;

		float		altitude = 0.0;

		float 		course_degrees = 0.0;
		float		speed_kmph = 0;

		// UTC date & time
		U32			lastValidDateTime_Time_ms = -1;
		DateTime	dateTime;

		// Time of the last data update
		U32 		lastUpdate_ms = 0;
	};

private:
	HardwareSerial&	m_serial;
	TinyGPSPlus		m_GPS;

	int				m_avgCount = 0;	// Counter of accumulated location values for the moving average

	bool			m_taskRunning = false;
	bool			m_killTask;		// A boolean used to finish the monitoring task
	QueueHandle_t 	m_queue;		// Lock-free global queue for R/W


public:

	// Immediate GPS data (not thread safe! Use GetGPSData() when using a monitor task)
	Data			m_data;

public:
	GPS( HardwareSerial& _serial ) : m_serial( _serial ) {
//		m_serial.onReceive( )

		m_queue = xQueueCreate( 1, sizeof(Data) );	// Size 1 = real time
	}

	// Start GPS task that will find fixes and update the GPS data on its own
	void	StartMonitoringTask() {
		xTaskCreatePinnedToCore(
			Task,
			"GPS Task",
			4096,	// Stack depth
			this,	// PV Parameters
			1,		// Priority
			NULL,	// Task handle
			1		// Core 1 (core 0 often used for WiFi)
		);
	}

	// Tells if the task is still running
	// It could have stopped for the following reason:
	//	• The user called KillTask()
	//	• The task failed to find the GPS module after 5s (can't monitor, need to check wiring)
	bool	IsTaskRunning() const { return m_taskRunning; }

	// Will kill the task as soon as possible...
	void	KillTask() { m_killTask = true; }

	// Attempts to read the GPS data from the lock-free queue
	// Returns true if the read was successful, or false if the read timed out
	bool	GetGPSData( Data& _data, U32 _timeOut_ms=1000 ) {
		return xQueueReceive( m_queue, &_data, pdMS_TO_TICKS(_timeOut_ms) );
	}

	// Callback to monitor the progress when searching for a fix
	// Returns true to continue the search, false to stop...
	typedef bool	(*FixProgressCallback)( const GPS::Data& _data, U32 _elapsedTime_ms, void* _parameter );

	// Wait for a satellite fix
	FIX_STATUS	FindFix( FixProgressCallback _Progresscallback, void* _parameter, U32 _timeOut_ms=-1 );

//	bool	isDateTimeValid() const { return m_lastValidDateTime_Time_ms != -1; }

	// Read GPS data from the module into the immediate structure (not thread safe!)
	void	ReadGPSData();


public:	// U-BLOX Specific methods (Typically used for the Neo-6M GPS module)

	// Computes the 2 bytes checksum of a data buffer
	// Source: section 3.4 of source u-blox documentation
	void	UBXChecksum( uint8_t* data, uint16_t len, uint8_t& ck_a, uint8_t& ck_b );

	// Sends a well-formatted UBX frame
	void	UBXSend( uint8_t cls, uint8_t id, uint8_t* payload, uint16_t len );

	// Waits for the module to acknowledge our command (Cf. sections 3.9.1 & 3.9.2)
	//	cls, the class of the command we need acknowledgment for
	//	id, the ID of the command we need acknowledgment for
	bool	UBXWaitForAck( uint8_t cls, uint8_t id, uint32_t timeout = 1000 );

	// Sends the initial position via the UBX-MGA-INI-POS_LLH frame (cf. section 3.12.8.2)
	//	_latitude and _longitude are in DEGREES
	void	UBXSendInitialPosition( double _latitude, double _longitude, double _altitude_m );

public:	// Helpers

	static void	Subtract( const RawDegrees& a, const RawDegrees& b, RawDegrees& result );

	// Computes the bearing angle in 0° (N) → 90° (E) → 180° (S) → 270° (W) and  the distance to the target in meters
	static float	ComputeDirection( double  _currentLatitude, double _currentLongitude, double _targetLatitude, double _targetLongitude, float& _distance_meters );

private:
	static void	Task( void* pvParameters );
	static bool	FindFixProgress( const GPS::Data& _data, U32 _elapsedTime_ms, void* _parameter );

	// Commits the data to the queue for any consumer to read
	void	CommitData() {
		xQueueOverwrite( m_queue, &m_data );	
	}
};
