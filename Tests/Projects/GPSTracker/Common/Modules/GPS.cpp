#include "GPS.h"

// Parallel task constantly monitoring GPS information and updating data in the thread-safe queue
bool	GPS::FindFixProgress( const GPS::Data& _data, U32 _elapsedTime_ms, void* _parameter ) {
	GPS*	gps = (GPS*) _parameter;
	gps->CommitData();	// Commit whatever is there...

//Serial.print( "." );

	return !gps->m_killTask;
}

void	GPS::Task( void* pvParameters ) {

	GPS*	that = (GPS*) pvParameters;

	// Start monitoring
	that->m_killTask = false;
	that->m_taskRunning = true;

	while ( !that->m_killTask ) {

		// Read any available characters & update GPS data
		that->ReadGPSData();

		// Check if we have a fix
		if ( !that->m_GPS.location.isValid() ) {
//Serial.println( "Find fix" );

			GPS::FIX_STATUS	status = that->FindFix( FindFixProgress, that );
			if ( status == GPS::FIX_STATUS::ERROR_NO_GPS_MODULE ) {
//Serial.println( "Killing task" );
				that->m_killTask = true;	// Exit task with an error...
			} else if ( status == GPS::FIX_STATUS::SEARCH_ABORTED_BY_USER ) {
				break;	// Exit task...
			} else if ( status != GPS::FIX_STATUS::FOUND_FIX ) {
				throw "Unhandled fix status!";
			}
		}

		// We have a fix!
		that->CommitData();	// Commit whatever is there...

		// Wait a bit
		vTaskDelay( pdMS_TO_TICKS(5) );
	}

//Serial.println( "Exiting task" );

	that->m_taskRunning = false;

	// Tasks can't return apparently...
	vTaskDelete( NULL );
}

GPS::FIX_STATUS	GPS::FindFix( FixProgressCallback _Progresscallback, void* _parameter, U32 _timeOut_ms ) {

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

	int	satellitesCount = m_data.satellitesCount;

	m_data.fixStatus = FIX_STATUS::NO_FIX;	// Start with no fix...
	while ( m_data.fixStatus != FIX_STATUS::FOUND_FIX && (now_ms - startTime_ms) < _timeOut_ms ) {
		ReadGPSData();

//Serial.printf( "Start %d - Now %d\r\n", startTime_ms, now_ms );

		if ( m_GPS.location.isValid() ) {
			// Found a fix!
			m_data.fixStatus = FIX_STATUS::FOUND_FIX;

			if ( m_data.satellitesCount > 0 ) {
				Serial.printf( "Found fix with %d satellites\r\n", m_data.satellitesCount );
			} else {
				Serial.printf( "Found fix. No satellites count info.\r\n" );
			}

			return m_data.fixStatus;
		}

		if ( now_ms - startTime_ms > 5000 && m_GPS.charsProcessed() < 10 ) {
			// No characters received from the module...
//Serial.println( "No GPS detected: check wiring." );
			m_data.fixStatus = FIX_STATUS::ERROR_NO_GPS_MODULE;
			return m_data.fixStatus;
		}

		delay( 100 );
		now_ms = millis();

		if ( _Progresscallback != nullptr, now_ms - lastProgress_ms > 1000 ) {
			lastProgress_ms = now_ms;
			if ( !(*_Progresscallback)( m_data, now_ms, _parameter ) ) {
				m_data.fixStatus = FIX_STATUS::SEARCH_ABORTED_BY_USER;
				return m_data.fixStatus;
			}
		}
	}

	// Time out!
	m_data.fixStatus = FIX_STATUS::ERROR_TIME_OUT;
	return m_data.fixStatus;
}

// Read GPS data while it's available
void	GPS::ReadGPSData() {
	if ( !m_serial.available() )
		return;	// Nothing to read...

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

		char	C = (char) m_serial.read();
//Serial.print( C );

		m_GPS.encode( C );
	}

	m_data.lastUpdate_ms = millis();

	// Read HDOP (our primary measure of quality)
	if ( m_GPS.hdop.isValid() ) {
		m_data.HDOP = m_GPS.hdop.hdop();
	} else {
		m_data.HDOP = 10;	// Very bad!
	}

	// Read satellites count
	if ( m_GPS.satellites.isValid() ) {
		m_data.satellitesCount = m_GPS.satellites.value();
	}

	// Extract location if available
	if ( m_GPS.location.isValid() ) {
		m_data.locationQuality = (Quality) m_GPS.location.FixQuality();

		// Read immediate value
		m_data.latitude = m_GPS.location.lat();
		m_data.longitude = m_GPS.location.lng();

// Conversion from raw degrees:
// double ret = m_latitude.deg + m_latitude.billionths / 1000000000.0;
// return m_latitude.negative ? -ret : ret;

		// Update exponential moving average
		if ( m_avgCount == 0 ) {
			// First value
			m_data.avgLatitude = m_data.latitude;
			m_data.avgLongitude = m_data.longitude;
		} else {
			// Adapt average speed depending on HDOP quality
			const float	MAX_HDOP = 10.0f;			// Worst quality (unusable)
			const float	TIME_PERIOD_WORST = 10;		// Small contribution when low quality
			const float	TIME_PERIOD_BEST = 2;		// Large contribution when high quality

			#if 1
				float	qualityFactor = 1.0f - min( 1.0f, m_data.HDOP / MAX_HDOP );	// 1 for good quality, 0 for bad quality
				float	timePeriod = TIME_PERIOD_WORST + qualityFactor * (TIME_PERIOD_BEST - TIME_PERIOD_WORST);
				float	blendFactor = 2.0f / (1 + timePeriod);				// Should yield 0.666 contribution for best signals (fast update, small smoothing) and 0.1818 for worst signals (slow update, large smoothing)
			#else
				float	blendFactor = EXPONENTIAL_MOVING_AVERAGE_FACTOR;	// Fixed, non adaptable
			#endif

			m_data.avgLatitude = m_data.latitude * blendFactor + m_data.avgLatitude * (1.0 - blendFactor);
			m_data.avgLongitude = m_data.longitude * blendFactor + m_data.avgLongitude * (1.0 - blendFactor);
		}
		m_avgCount++;

		m_data.lastValidLocation_Time_ms = m_data.lastUpdate_ms;
	}

	// Extract minor information (altitude, course & speed)
	if ( m_GPS.altitude.isValid() ) {
		m_data.altitude = m_GPS.altitude.meters();
	}
	if ( m_GPS.course.isValid() ) {
		m_data.course_degrees = m_GPS.course.deg();
	}
	if ( m_GPS.speed.isValid() ) {
		m_data.speed_kmph = m_GPS.speed.kmph();
	}

// So apparently we can get a "valid" time and date that is clearly wrong, even without a location fix...
// I think it's best to wait for a proper satellite fix before reading the date & time! (it can take a while though :/)
//
	// Extract UTC date & time if available
	if ( m_GPS.time.isValid() && m_GPS.date.isValid() ) {
		m_data.dateTime.Y = m_GPS.date.year();
		m_data.dateTime.M = m_GPS.date.month();
		m_data.dateTime.D = m_GPS.date.day();
		m_data.dateTime.h = m_GPS.time.hour();
		m_data.dateTime.m = m_GPS.time.minute();
		m_data.dateTime.s = m_GPS.time.second();

		m_data.lastValidDateTime_Time_ms = m_data.lastUpdate_ms;
	}
}

// Converts a UTC time into a local PST time
time_t	gpsToUnixUTC( struct tm& t ) {
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

void	GPS::UBXChecksum( uint8_t* data, uint16_t len, uint8_t& ck_a, uint8_t& ck_b ) {
	ck_a = 0;
	ck_b = 0;

	for ( U16 i=0; i < len; i++ ) {
		ck_a += data[i];
		ck_b += ck_a;
	}
}

void	GPS::UBXSend( uint8_t cls, uint8_t id, uint8_t* payload, uint16_t len ) {
	// [SYNC][CLASS][ID][LENGTH][PAYLOAD][CK_A][CK_B]
	// 
	//	SYNC = 0xB5 0x62
	//	CLASS = ex: 0x13 (MGA)
	//	ID = ex: 0x40 (INI)
	//	LENGTH = payload size (little endian)
	//	PAYLOAD = data
	//	CHECKSUM = computed
	uint8_t ck_a, ck_b;

	uint8_t	header[4] = { cls, id, (uint8_t)(len & 0xFF), (uint8_t)(len >> 8) };
	UBXChecksum( header, 4, ck_a, ck_b );
	UBXChecksum( payload, len, ck_a, ck_b );

	m_serial.write( 0xB5 );
	m_serial.write( 0x62 );

	m_serial.write( header, 4 );
	m_serial.write( payload, len );

	m_serial.write( ck_a );
	m_serial.write( ck_b );
}

bool	GPS::UBXWaitForAck( uint8_t cls, uint8_t id, uint32_t timeout ) {
	uint32_t	start = millis();

	uint8_t	step = 0;
	uint8_t	ackClass = 0, ackID = 0;

	while ( millis() - start < timeout ) {
		if ( !m_serial.available() ) {
			delay( 1 );
			continue;
		}

		uint8_t	b = m_serial.read();

		switch ( step ) {
			// Expect the 0x85 0x62 header
			case 0: if ( b == 0xB5 ) step++; break;
			case 1: if ( b == 0x62 ) step++; else step = 0; break;

			// Expect USB-ACK class ID (0x05)
			case 2: if ( b == 0x05 ) step++; else step = 0; break;

			// Expect ACK (0x01) or NAK msg ID (0x00)
			case 3:
				if ( b == 0x01 || b == 0x00 ) {
					ackID = b; // ACK or NAK
					step++;
				} else step = 0;
				break;

			// Expect payload length 2 (U16)
			case 4: if ( b == 0x02 ) step++; else step = 0; break;
			case 5: if ( b == 0x00 ) step++; else step = 0; break;

			// Acknowledegd class ID
			case 6: ackClass = b; step++; break;
			// Acknowledged message ID
			case 7: ackID = b; step++; break;

			// Ignore checksum
			case 8: step++; break;
			case 9:
				// Message end
				if ( ackClass == cls && ackID == id ) {
					return true; // ACK reçu
				}

				// Wrong class and/or message IDs
				step = 0;
				break;
		}
	}

	return false; // Timeout or NAK
}

void	GPS::UBXSendInitialPosition( double _latitude, double _longitude, double _altitude_m ) {
	uint8_t	payload[20] = {0};

	int32_t	lat = _latitude * 1e7;
	int32_t	lon = _longitude * 1e7;
	int32_t	alt = _altitude_m * 100; // m → cm

	payload[0] = 0x01; // type = position

	memcpy( payload + 4, &lat, 4 );
	memcpy( payload + 8, &lon, 4 );
	memcpy( payload + 12, &alt, 4 );

	uint32_t positionAccuracy = 10000; // 10 m
	memcpy( payload + 16, &positionAccuracy, 4 );

	// Send UBX-MGA-INI-POS_LLH
	UBXSend( 0x13, 0x40, payload, 20 );
}
