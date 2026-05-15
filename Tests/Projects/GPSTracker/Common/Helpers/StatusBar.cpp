#include "StatusBar.h"

void	StatusBar::Update( BATTERY_CHARGE _batteryCharge ) {
	m_batteryCharge = _batteryCharge;
	Update();
}

void	StatusBar::Update( WIFI_STRENGTH _wifiStrength ) {
	m_wifiStrength = _wifiStrength;
	Update();
}

void	StatusBar::Update( GPS_SIGNAL_STRENGTH _GPSSignalStrength ) {
	m_GPSSignalStrength = _GPSSignalStrength;
	Update();
}

void	StatusBar::SetRect( S16 X, S16 Y, U16 _width, U16 _height ) {
	m_left = X;
	m_top = Y;
	m_width = _width;
	m_height = _height;
	m_display.m_tft.fillRect( m_left, m_top, m_width, m_height, TFT_BLACK );
}

void	StatusBar::Update() {
	S16	X = m_marginX;
	S16	Y = m_marginY;
	m_display.DrawBitmap( m_icons[m_iconRangeStartIndexBatteryCharge + int(m_batteryCharge) + 1], X, Y );		X += 16;
	m_display.DrawBitmap( m_icons[m_iconRangeStartIndexWifiStrength + int(m_wifiStrength)], X, Y );				X += 16;
	m_display.DrawBitmap( m_icons[m_iconRangeStartIndexGPSSignalStrength + int(m_GPSSignalStrength)], X, Y );	X += 16;
}
