#pragma once

#include "../Global.h"

#include <TinyGPSPlus.h>

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
	bool			m_lostFix;

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
		m_lostFix = false;
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
	bool	FindFix( U32 _timeOut_ms=-1 );

	double	lat() const;
	double	lng() const;

	bool	isDateTimeValid() const { return m_lastValidDateTime_Time_ms != -1; }

	void	ReadGPSData();

public:	// Helpers
	static void	Subtract( const RawDegrees& a, const RawDegrees& b, RawDegrees& result );
};