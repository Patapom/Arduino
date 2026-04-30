#include "GPS.h"

bool	GPS::FindFix( U32 _timeOut_ms ) {

#if 0 // Basic serial printing of GPS data
	if ( Serial1.available() ) {
		Serial.print( (char) Serial1.read() );
	}
	return;
#endif

//	if ( Serial1.available() == 0 || !GPS.encode( Serial1.read() ) ) {
//		if ( (millis() - startTime_ms) > 5000 && GPS.charsProcessed() < 10 ) {
//			display.println( "No GPS detected: check wiring." );
//			while(true);
//		}
//		delay( 100 );
//		return;
//	}

	U32	startTime_ms = millis();
	U32	now_ms = startTime_ms;

	m_hasFix = false;
	while ( !m_hasFix && now_ms - startTime_ms < _timeOut_ms ) {
		ReadGPSData();

		if ( m_GPS.location.isValid() ) {
			m_hasFix = true;
			break;
		}

		if ( now_ms - startTime_ms > 5000 && m_GPS.charsProcessed() < 10 ) {
			// No characters received from the module...
			Serial.println( "No GPS detected: check wiring." );
			return false;
		}

		delay( 100 );
		now_ms = millis();
	}

	return m_hasFix;
}

double	GPS::lat() const {
   double ret = m_latitude.deg + m_latitude.billionths / 1000000000.0;
   return m_latitude.negative ? -ret : ret;
}

double	GPS::lng() const {
	double ret = m_longitude.deg + m_longitude.billionths / 1000000000.0;
	return m_longitude.negative ? -ret : ret;
}

// Read GPS data while it's available
void	GPS::ReadGPSData() {
	while ( m_serial.available() ) {
		m_GPS.encode( m_serial.read() );
	}

	// Extract location if available
	if ( m_GPS.location.isValid() ) {
		m_locationQuality = (Quality) m_GPS.location.FixQuality();
		m_latitude = m_GPS.location.rawLat();
		m_longitude = m_GPS.location.rawLng();

		m_lastValidLocation_Time_ms = millis();
	}

// So apparently we can get a "valid" time and date that is clearly wrong, even without a location fix...
// I think it's best to wait for a proper satellite fix before reading the date & time! (it can take a while though :/)
//
	// Extract UTC date & time if available
	if ( m_GPS.time.isValid() && m_GPS.date.isValid() ) {
		m_dateTime.Y = m_GPS.date.year();
		m_dateTime.M = m_GPS.date.month();
		m_dateTime.D = m_GPS.date.day();
		m_dateTime.h = m_GPS.time.hour();
		m_dateTime.m = m_GPS.time.minute();
		m_dateTime.s = m_GPS.time.second();

		m_lastValidDateTime_Time_ms = millis();
	}
}

// Converts a UTC time into a local PST time
time_t gpsToUnixUTC( struct tm& t ) {
	static const int days_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};

	int year = t.tm_year + 1900;
	int month = t.tm_mon + 1;

	// années depuis 1970
	long days = 0;
	for (int y = 1970; y < year; y++) {
		days += ( (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0) ) ? 366 : 365;
	}

	for (int m = 1; m < month; m++) {
		days += days_month[m-1];
		if (m == 2 && ((year%4==0 && year%100!=0) || (year%400==0)))
			days += 1;
	}

	days += (t.tm_mday - 1);

	return days * 86400
			+ t.tm_hour * 3600
			+ t.tm_min * 60
			+ t.tm_sec;
}

void	GPS::DateTime::ToLocal( DateTime& _localDateTime ) {

	struct tm	utc;
				utc.tm_year = Y - 1900;
				utc.tm_mon  = M - 1;
				utc.tm_mday = D;
				utc.tm_hour = h;
				utc.tm_min  = m;
				utc.tm_sec  = s;

	// UTC time zone
	setenv( "TZ", "UTC0", 1 );
	tzset();

	// Convert into timestamp
//	time_t		t = timegm( &t );	// Non standard... Doesn't exist on ESP32
//	time_t		t = mktime( &utc );
	time_t		t = gpsToUnixUTC( utc );

	// Vancouver Time Zone
	setenv( "TZ", "PST8PDT,M3.2.0,M11.1.0", 1 );
	tzset();

	struct tm*	local = localtime( &t );
				local->tm_year += 1900;
				local->tm_mon++;

	_localDateTime.Y = local->tm_year;
	_localDateTime.M = local->tm_mon;
	_localDateTime.D = local->tm_mday;

	_localDateTime.h = local->tm_hour;
	_localDateTime.m = local->tm_min;
	_localDateTime.s = local->tm_sec;
}

void	GPS::Subtract( const RawDegrees& a, const RawDegrees& b, RawDegrees& result ) {
	S16	degA = S16( a.deg );
	if ( a.negative )
		degA = -degA;
	S32	billionthA = S32( a.billionths );

	S16	degB = S16( b.deg );
	if ( b.negative )
		degB = -degB;
	S32	billionthB = S32( b.billionths );

	S32	deltaDeg = degA - degB;
	S32	deltaBilionths = billionthA - billionthB;
	if ( deltaBilionths < 0 ) {
		deltaDeg--;
		deltaBilionths += 1000000000;
	} else if ( deltaBilionths >= 1000000000 ) {
		deltaDeg--;
		deltaBilionths -= 1000000000;
	}

	result.negative = deltaDeg < 0;
	result.deg = deltaDeg < 0 ? -deltaDeg : deltaDeg;
	result.billionths = deltaBilionths;
}
