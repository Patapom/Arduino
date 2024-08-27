// High-Speed Serial Interface with the PC
#pragma once

//#include <esp32-hal-uart.h>
#include "../AudioBuffer.h"
#include "../ESPNow/TransportESPNow.h"

#if 0	// Redefine error reports locally
extern void	CENTRAL_SERIAL_ERROR( bool _setError, const char* _functionName, const char* _message );
#undef ERROR
#define ERROR( _setError, _message ) CENTRAL_SERIAL_ERROR( _setError, __func__, _message )
#endif

class CentralSerial {
public:
	static const U32	SAMPLES_PER_PACKET = 256;

	enum PORT {
		UART0 = 0,	// UART_NUM_0
		UART1 = 1,	// UART_NUM_1
		UART2 = 2,	// UART_NUM_2
	};

public:
	const ITimeReference&	m_time;

	PORT	m_port;

	// Peripherals -> UART
	ISampleSource*	m_sampleSource = NULL;	// The audio source we're sampling when auto-transmitting to UART
	Sample			m_buffer[SAMPLES_PER_PACKET];

	// UART -> Peripherals
	TransportESPNow_Transmitter*	m_transmitter = NULL;	// The transmitter we're forwarding to when auto-receiving from UART
	U8				m_packet[ESP_NOW_MAX_DATA_LEN];

	U32		m_invalidPacketsCount = 0;
	U32		m_sentPacketsCount = 0;
	U32		m_searchedBytesCount = 0;

public:
	CentralSerial( const ITimeReference& _time ) : m_time( _time ) {}
	
	void	Init( PORT _port, U32 _baudRate, U8 _pinTX, U8 _pinRX );

	// Starts an auto-send task to send packets at the required sampling rate
	void	StartAutoSendTask( U8 _taskPriority, ISampleSource& _sampleSource );

	void	StartAutoReceiveTask( U8 _taskPriority, TransportESPNow_Transmitter& _transmitter );

	// Sends a single packet of samples through the serial
	void	SendPacket( ISampleSource& _source );

public:
	void	begin( int ) {}
	void	println( const char* a = NULL ) {}
	void	printf( const char* a, ... ) {}

private:
	U32		m_packetOffset = 0;
	U32		m_packetIDAudio = 0;
	U32		m_packetIDCommand = 0;
	bool	m_searchMode = true;	// Tells if we're in search mode or burst mode. Search mode happens when received data are desynchronized with packet headers

	void	ProcessBlock( const U8* _data, U32 _blockSize );
	void	ProcessBlock2( const U8* _data, U32 _blockSize );
	void	ResetPacket();

	friend void	SerialSendPacketsTask( void* _param );
	friend void	SerialReceivePacketsTask( void* _param );
};

#ifdef NO_GLOBAL_SERIAL
extern CentralSerial	Serial;
#endif