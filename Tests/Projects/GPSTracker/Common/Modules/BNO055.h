#include "../Global.h"

#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


void	ShowMagnetometerData();
void	GetCalibration();
bool	ShouldCalibrate();
void	Calibrate();
void	ScanI2C();
