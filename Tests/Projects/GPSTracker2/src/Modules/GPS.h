#pragma once

#include "Global.h"
#include <TinyGPSPlus.h>

class	GPS {
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

	// A copy of the TinyGPS version
	enum Quality { Invalid = '0', OGPS = '1', DGPS = '2', PPS = '3', RTK = '4', FloatRTK = '5', Estimated = '6', Manual = '7', Simulated = '8' };

private:
	HardwareSerial&	m_serial;
	TinyGPSPlus		m_GPS;

public:

	bool			m_hasFix;
	bool			m_lostFix;

	// Location
	U32				m_lastValidLocation_Time_ms;
	Quality			m_locationQuality;
	RawDegrees		m_latitude;
	RawDegrees		m_longitude;

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