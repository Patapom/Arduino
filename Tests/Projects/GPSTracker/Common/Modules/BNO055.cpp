#include "BNO055.h"
/*
Adafruit_BNO055	bno = Adafruit_BNO055( 55, 0x29 );

void	ShowMagnetometerData() {
	// Lecture des angles (Euler)
	imu::Vector<3> euler = bno.getVector( Adafruit_BNO055::VECTOR_EULER );

	float	heading = euler.x(); // Cap (boussole)
	float	roll    = euler.z();
	float	pitch   = euler.y();

	float 	bearing = bno.getTemp();
	int 	temp = bno.getTemp();

	Serial.print( "Cap: " );
	Serial.print( heading );
	Serial.print( "°  | Pitch: " );
	Serial.print( pitch );
	Serial.print( "°  | Roll: " );
	Serial.print( roll );
	Serial.print( "°" );
	Serial.printf( " | Bearing: %.2f° | Temp: %d°C\r\n", bearing, temp );
}

void	GetCalibration() {

	// Show mode
	adafruit_bno055_opmode_t	operatingMode = bno.getMode();
	const char*					strMode = NULL;
	switch ( operatingMode ) {
  		case OPERATION_MODE_CONFIG:		strMode = "OPERATION_MODE_CONFIG"; break;
  		case OPERATION_MODE_ACCONLY:	strMode = "OPERATION_MODE_ACCONLY"; break;
  		case OPERATION_MODE_MAGONLY: 	strMode = "OPERATION_MODE_MAGONLY"; break;
  		case OPERATION_MODE_GYRONLY: 	strMode = "OPERATION_MODE_GYRONLY"; break;
  		case OPERATION_MODE_ACCMAG: 	strMode = "OPERATION_MODE_ACCMAG"; break;
  		case OPERATION_MODE_ACCGYRO: 	strMode = "OPERATION_MODE_ACCGYRO"; break;
  		case OPERATION_MODE_MAGGYRO:	strMode = "OPERATION_MODE_MAGGYRO"; break;
  		case OPERATION_MODE_AMG: 		strMode = "OPERATION_MODE_AMG"; break;
  		case OPERATION_MODE_IMUPLUS:	strMode = "OPERATION_MODE_IMUPLUS"; break;
  		case OPERATION_MODE_COMPASS:	strMode = "OPERATION_MODE_COMPASS"; break;
  		case OPERATION_MODE_M4G: 		strMode = "OPERATION_MODE_M4G"; break;
  		case OPERATION_MODE_NDOF_FMC_OFF: strMode = "OPERATION_MODE_NDOF_FMC_OFF"; break;
  		case OPERATION_MODE_NDOF: 		strMode = "OPERATION_MODE_NDOF"; break;
	}
	Serial.printf( "BNO operating mode: %s\r\n", strMode );

	// Show calibration values
	uint8_t sys, gyro, accel, mag;
	bno.getCalibration( &sys, &gyro, &accel, &mag );

	Serial.print("CALIB SYS:");
	Serial.print(sys);
	Serial.print(" G:");
	Serial.print(gyro);
	Serial.print(" A:");
	Serial.print(accel);
	Serial.print(" M:");
	Serial.println(mag);
/// valeurs de 0 à 3
/// pour une bonne boussole → mag = 3
}

bool	ShouldCalibrate() {
	uint8_t sys, gyro, accel, mag;
	bno.getCalibration( &sys, &gyro, &accel, &mag );

	return mag < 3;
}

void	Calibrate() {
	uint8_t sys, gyro, accel, mag = 0;

	while ( mag < 3 ) {
		bno.getCalibration( &sys, &gyro, &accel, &mag );
		Serial.printf( "Calibrating... sys=%d gyro=%d accel=%d mag=%d\r\n", sys, gyro, accel, mag );
		delay( 250 );
	}

	// Read offsets (store for next time)
	adafruit_bno055_offsets_t	offsets;
	bno.getSensorOffsets( offsets );
//	bno.setSensorOffsets( offsets );
	Serial.printf( "Final offsets: \r\n" );
	Serial.printf( " → Accel = %d, %d, %d (radius = %d)\r\n", offsets.accel_offset_x, offsets.accel_offset_y, offsets.accel_offset_z, offsets.accel_radius );
	Serial.printf( " → Mag   = %d, %d, %d (radius = %d)\r\n", offsets.mag_offset_x, offsets.mag_offset_y, offsets.mag_offset_z, offsets.mag_radius );
	Serial.printf( " → Gyro  = %d, %d, %d\r\n", offsets.gyro_offset_x, offsets.gyro_offset_y, offsets.gyro_offset_z );

// Final offsets: 
//  → Accel = 16379, 8652, 16379 (radius = 7440)
//  → Mag   = 29597, -32755, 8800 (radius = 16380)
//  → Gyro  = 16379, 8088, 16380
}

// Restores the offsets found by the calibration
void	RestoreOffsets() {
//  → Accel = 16379, 8652, 16379 (radius = 7440)
//  → Mag   = 29597, -32755, 8800 (radius = 16380)
//  → Gyro  = 16379, 8088, 16380
	adafruit_bno055_offsets_t	offsets;
	offsets.accel_offset_x = 16379, 8652, 16379;
	offsets.accel_offset_y = 16379, 8652, 16379;
	offsets.accel_offset_z = 16379, 8652, 16379;
	offsets.accel_radius = 16379, 8652, 16379;
	offsets.gyro_offset_x = 16379, 8652, 16379;

	bno.getSensorOffsets( offsets );
}

void	ScanI2C() {
	Serial.println( "Scan I2C..." );
	bool	found = false;
	for ( byte addr=1; addr < 127; addr++ ) {
		Wire.beginTransmission( addr );
		int	error = Wire.endTransmission();
		if ( error == 0 ) {
			found = true;	// Found at least 1 device!

			Serial.print( "Device found at 0x" );
			Serial.println( addr, HEX );
		} else if (error==4) {
			Serial.print("Unknow error at address 0x");
			if ( addr  < 16 ) {
				Serial.print( "0" );
			}
			Serial.println( addr, HEX );
		}
	}
	if ( !found )
		Serial.println( "Scan failed: No device found!" );
}
*/
