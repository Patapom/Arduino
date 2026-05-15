#pragma once

#include "../Global.h"
#include "Display.h"
#include "IconsPalette.h"

class StatusBar {
public:

	enum class BATTERY_CHARGE {
		CHARGING = -1,
		DEPLETED = 0,
		CHARGE25 = 1,
		CHARGE50 = 2,
		CHARGE75 = 3,
		CHARGE100 = 4,
	};

	enum class WIFI_STRENGTH {
		NO_WIFI = 0,
		STRENGTH25 = 1,
		STRENGTH50 = 2,
		STRENGTH75 = 3,
		STRENGTH100 = 4,
	};

	enum class GPS_SIGNAL_STRENGTH {
		NO_SIGNAL = 0,
		STRENGTH25 = 1,
		STRENGTH50 = 2,
		STRENGTH75 = 3,
		STRENGTH100 = 4,
	};

private:
	TFTDisplay&			m_display;
	const IconsPalette&	m_icons;

	S16					m_left = 0, m_top = 0;
	U16					m_width = 100, m_height = 20;
	U8					m_marginX = 0, m_marginY = 0;

	U8					m_iconRangeStartIndexBatteryCharge = 0;
	U8					m_iconRangeStartIndexWifiStrength = 0;
	U8					m_iconRangeStartIndexGPSSignalStrength = 0;

	BATTERY_CHARGE		m_batteryCharge = BATTERY_CHARGE::DEPLETED;
	WIFI_STRENGTH		m_wifiStrength = WIFI_STRENGTH::NO_WIFI;
	GPS_SIGNAL_STRENGTH	m_GPSSignalStrength = GPS_SIGNAL_STRENGTH::NO_SIGNAL;

public:
	StatusBar( TFTDisplay& _display, const IconsPalette& _icons ) : m_display( _display ), m_icons( _icons ) {}
	~StatusBar() {}

	void	SetRect( S16 X, S16 Y, U16 _width, U16 _height ) { m_left = X; m_top = Y; m_width = _width; m_height = _height; }
	void	SetMargin( U8 _marginX, U8 _marginY ) { m_marginX = _marginX; m_marginY = _marginY; }

	void	Update( BATTERY_CHARGE _batteryCharge );
	void	Update( WIFI_STRENGTH _wifiStrength );
	void	Update( GPS_SIGNAL_STRENGTH _GPSSignalStrength );

	void	SetIconsRangeBatteryCharge( U8 _rangeStartIndex ) { m_iconRangeStartIndexBatteryCharge = _rangeStartIndex; }
	void	SetIconsRangeWifiStrength( U8 _rangeStartIndex ) { m_iconRangeStartIndexWifiStrength = _rangeStartIndex; }
	void	SetIconsRangeGPSSignalStrength( U8 _rangeStartIndex ) { m_iconRangeStartIndexGPSSignalStrength = _rangeStartIndex; }

private:
	void	Update();
};
