#include "GPS.h"

// Inspired by Robojax's code (https://robojax.com/learn/arduino/?vid=robojax_GPS_TinyGPSPlus)
//
GPS::FIX_STATUS	GPS::FindFix( U32 _timeOut_ms ) {

#if 0 // Basic serial printing of GPS data
	while ( true ) {
		if ( m_serial.available() ) {
			Serial.print( (char) Serial1.read() );
		} else {
			delay( 1 );
		}
	}
#endif

	U32	startTime_ms = millis();
	U32	now_ms = startTime_ms;
	U32	lastProgress_ms = now_ms;

	m_hasFix = false;
	m_satellitesCount = 0;
	while ( !m_hasFix && (now_ms - startTime_ms) < _timeOut_ms ) {
		ReadGPSData();

//Serial.printf( "Start %d - Now %d\r\n", startTime_ms, now_ms );

		if ( m_GPS.satellites.isValid() && m_GPS.satellites.value() != m_satellitesCount ) {
			m_satellitesCount = m_GPS.satellites.value();
Serial.printf( "Satellites count %d\r\n", m_satellitesCount );
		}

		if ( m_GPS.location.isValid() ) {
			// Found a fix!
			m_hasFix = true;
			break;
		}

		if ( now_ms - startTime_ms > 5000 && m_GPS.charsProcessed() < 10 ) {
			// No characters received from the module...
			Serial.println( "No GPS detected: check wiring." );
			return FIX_STATUS::ERROR_NO_GPS_MODULE;
		}

		delay( 100 );
		now_ms = millis();

		if ( now_ms - lastProgress_ms > 1000 ) {
			lastProgress_ms = now_ms;
			Serial.print( "." );
		}
	}

	if ( m_GPS.satellites.isValid() ) {
		m_satellitesCount = m_GPS.satellites.value();
		Serial.printf( "Found fix with %d satellites\r\n", m_satellitesCount );
	} else {
		Serial.printf( "Found fix. No satellites count info.\r\n" );
	}

	return m_hasFix ? FIX_STATUS::FOUND_FIX : FIX_STATUS::ERROR_TIME_OUT;
}

// Read GPS data while it's available
void	GPS::ReadGPSData() {
	while ( m_serial.available() ) {
		// example received:
		//	GPGSV,4,3,14,21,12,321,22,26,25,048,16,27,04,112,,29,00,003,*78
		//	$GPGSV,4,4,14,30,08,244,,31,05,062,11*76
		//	$GPGLL,4930.96847,N,12421.72920,W,220821.00,A,A*70
		//	$GPRMC,220822.00,A,4930.96861,N,12421.72885,W,0.465,,300925,,,A*6A
		//	$GPVTG,,T,,M,0.465,N,0.861,K,A*2B
		//	$GPGGA,220822.00,4930.96861,N,12421.72885,W,1,09,0.96,72.8,M,-17.5,M,,*59
		//	$GPGSA,A,3,16,31,03,09,26,21,04,07,11,,,,1.56,0.96,1.23*04
		//	$GPGSV,4,1,14,03,22,171,18,04,63,102,20,06,14,264,,07,38,244,14*7C
		//	$GPGSV,4,2,14,09,72,301,09,11,15,303,12,16,48,088,26,20,12,324,*71
		//	$GPGSV,4,3,14,21,12,321,23,26,25,048,16,27,04,112,,29,00,003,*79
		//	$GPGSV,4,4,14,30,08,244,,31,05,062,11*76
		//	$GPGLL,4930.96861,N,12421.72885,W,220822.00,A,A*79
		//	$GPRMC,220823.00,A,4930.96871,N,12421.72858,W,0.301,,300925,,,A*6F
		//	$GPVTG,,T,,M,0.301,N,0.558,K,A*29
		//	$GPGGA,220823.00,4930.96871,N,12421.72858,W,1,09,0.96,72.9,M,-17.5,M,,*58
		//	$GPGSA,A,3,16,31,03,09,26,21,04,07,11,,,,1.56,0.96,1.23*04
		//	$GPGSV,4,1,14,03,22,171,18,04,63,102,19,06,14,264,,07,38,244,14*76
		//	$GPGSV,4,2,14,09,72,301,09,11,15,303,12,16,48,088,26,20,12,324,*71
		//	$GPGSV,4,3,14,21,12,321,22,26,25,048,17,27,04,112,,29,00,003,*79
		//	$GPGSV,4,4,14,30,08,244,,31,05,062,11*76
		//	$GPGLL,4930.96871,N,12421.72858,W,220823.00,A,A*79
		//	$GPRMC,220824.00,A,4930.96893,N,12421.72849,W,0.234,,300925,,,A*63
		//	$GPVTG,,T,,M,0.234,N,0.433,K,A*22
		//	$GPGGA,220824.00,4930.96893,N,12421.72849,W,1,09,0.96,73.0,M,-17.5,M,,*5B
		//	$GPGSA,A,3,16,31,03,09,26,21,04,07,11,,,,1.56,0.96,1.23*04
		//	$GPGSV,4,1,14,03,22,171,18,04,63,102,19,06,14,264,,07,38,244,14*76
		//	$GPGSV,4,2,14,09,72,301,09,11,15,303,12,16,48,088,26,20,12,324,*71
		//	$GPGSV,4,3,14,21,12,321,22,26,25,048,16,27,04,112,,29,00,003,*78
		//	$GPGSV,4,4,14,30,08,244,,31,05,062,11*76
		//	$GPGLL,4930.96893,N,12421.72849,W,220824.00,A,A*72

		m_GPS.encode( m_serial.read() );
	}

	// Extract location if available
	if ( m_GPS.location.isValid() ) {
		m_locationQuality = (Quality) m_GPS.location.FixQuality();

		m_latitude = m_GPS.location.lat();
		m_longitude = m_GPS.location.lng();

// Conversion from raw degrees:
// double ret = m_latitude.deg + m_latitude.billionths / 1000000000.0;
// return m_latitude.negative ? -ret : ret;

		// Update exponential moving average
		if ( m_avgCount == 0 ) {
			m_avgLatitude = m_latitude;
			m_avgLongitude = m_longitude;
		} else {
			m_avgLatitude = m_latitude * EXPONENTIAL_MOVING_AVERAGE_FACTOR + m_avgLatitude * (1.0 - EXPONENTIAL_MOVING_AVERAGE_FACTOR);
			m_avgLongitude = m_longitude * EXPONENTIAL_MOVING_AVERAGE_FACTOR + m_avgLongitude * (1.0 - EXPONENTIAL_MOVING_AVERAGE_FACTOR);
		}
		m_avgCount++;

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

Serial.printf( "A = %d.%09d\r\n", degA, billionthA );
Serial.printf( "B = %d.%09d\r\n", degB, billionthB );
Serial.printf( "delta = %d.%09d\r\n", deltaDeg, deltaBilionths );

	if ( deltaBilionths < 0 ) {
		deltaDeg--;
		deltaBilionths += 1000000000;
	} else if ( deltaBilionths >= 1000000000 ) {
		deltaDeg--;
		deltaBilionths -= 1000000000;
	}

//A = 49.516243333
//B = 49.516100000
//delta = 0.000143333
//
//A = -124.362251333
//B = -124.362500000
//delta = 0.-00248667
//Delta lat = 0.000143333 Delta lon = -1.999751333


	result.negative = deltaDeg < 0;
	result.deg = deltaDeg < 0 ? -deltaDeg : deltaDeg;
	result.billionths = deltaBilionths;
}

const double	EARTH_RADIUS_METERS = 6371000.0;

float	GPS::ComputeDirection( double  _currentLatitude, double _currentLongitude, double _targetLatitude, double _targetLongitude, float& _distance_meters ) {
	_currentLatitude *= DEG_TO_RAD;
	_currentLongitude *= DEG_TO_RAD;
	_targetLatitude *= DEG_TO_RAD;
	_targetLongitude *= DEG_TO_RAD;

	double	lat1 = _currentLatitude;
	double	lat2 = _targetLatitude;
	double	deltaLat = _targetLatitude - _currentLatitude;
	double	deltaLon = _targetLongitude - _currentLongitude;

	// Haversine formula
	//	• Compute the angle a of the arc between the current & target locations
	//	• Return the perimeter of the great arc subtending this angle
	//
	double a =	sin(deltaLat/2) * sin(deltaLat/2) +
				sin(deltaLon/2) * sin(deltaLon/2) * cos(lat1) * cos(lat2);

	double arcLength = 2 * EARTH_RADIUS_METERS * asin( sqrt( max( 0.0, a ) ) );
//	double arcLength = 2 * EARTH_RADIUS_METERS * atan2( sqrt( a ), sqrt( 1-a ) );

	_distance_meters = float( arcLength );

	// Compute bearing
	double	y = sin(deltaLon) * cos(lat2);
	double	x = cos(lat1) * sin(lat2) -
				sin(lat1)*cos(lat2)*cos(deltaLon);

	double	bearing = atan2( y, x ) * RAD_TO_DEG;
	if ( bearing < 0 ) bearing += 360;	// Normalisation 0–360

	return float( bearing );
}
