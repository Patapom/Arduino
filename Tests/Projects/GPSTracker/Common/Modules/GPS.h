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
		UNKNOWN = -1,
		FOUND_FIX,				// Successfully found a fix (read Quality field and satellites count for overall quality)
		ERROR_TIME_OUT,			// The search for a fix timed out without success
		ERROR_NO_GPS_MODULE,	// Returns failure after 5s without any life sign from the 
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

private:
	HardwareSerial&	m_serial;
	TinyGPSPlus		m_GPS;

public:

	bool			m_hasFix;
	U32				m_satellitesCount = 0;

	// Location
	U32				m_lastValidLocation_Time_ms;
	Quality			m_locationQuality;

	// We're storing latitudes and longitudes as 64-bits double precision which should be enough to account
	//	for the precision of the raw measures sent by the module which can go up to 1e-9 units
	// If we're assuming degree values between -180 and +180 degrees and 9 digits of precision, we need
	// 	to store a number with an magnitude of 1e12 = 39.86 bits of precision
	// We know 64-bits floats use 53 bits of mantissa and 11 bits of exponent, which is largely enough
	//	to accommodate our needs here...
	//
	double			m_latitude;
	double			m_longitude;

	// These values are accumulated using an exponential moving average (EMA) with a common smoothing length of 10 values
	double			m_avgLatitude;
	double			m_avgLongitude;
	int				m_avgCount;

	// UTC date & time
	U32				m_lastValidDateTime_Time_ms;
	DateTime		m_dateTime;

public:
	GPS( HardwareSerial& _serial ) : m_serial( _serial ) {
		m_hasFix = false;
		m_satellitesCount = 0;
		m_lastValidLocation_Time_ms = -1;
		m_lastValidDateTime_Time_ms = -1;

		m_locationQuality = Invalid;

		// Initialze running average
		m_avgLatitude = 0;
		m_avgLongitude = 0;
		m_avgCount = 0;
//		m_serial.onReceive( )
	}

	// Wait for a satellite fix
	FIX_STATUS	FindFix( U32 _timeOut_ms=-1 );

	double	lat() const;
	double	lng() const;

	bool	isDateTimeValid() const { return m_lastValidDateTime_Time_ms != -1; }

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
};