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

public:
	CentralSerial( const ITimeReference& _time ) : m_time( _time ) {}
	
	void	Init( PORT _port, U32 _baudRate, U8 _pinTX, U8 _pinRX );

	// Starts an auto-send task to send packets at the required sampling rate
	void	StartAutoSendTask( U8 _taskPriority, ISampleSource& _sampleSource );

	void	StartAutoReceiveTask( U8 _taskPriority, TransportESPNow_Transmitter& _transmitter );

	// Sends a single packet of samples through the serial
	void	SendPacket( ISampleSource& _source );

//	void	Read();
//	void	Write();

public:
	void	begin( int ) {}
	void	println( const char* a = NULL ) {}
	void	printf( const char* a, ... ) {}

private:
//	enum PROCESS_STATE {
//		PS_UNKNOWN = -1,
//		PS_AWAITING_HEADER_BYTE0 = 0,
//		PS_AWAITING_HEADER_BYTE1 = 1,
//		PS_AWAITING_HEADER_RECEIVER_MASK = 2,
//		PS_AWAITING_PACKET_LENGTH = 3,
//		PS_COPYING = 4,
//	};
//	PROCESS_STATE	m_processState = PS_AWAITING_HEADER_BYTE0;
	U32				m_packetOffset = 0;
	U32				m_packetIDAudio = 0;
	U32				m_packetIDCommand = 0;
	void	ProcessBlock( const U8* _data, U32 _blockSize );
	void	ProcessBlock2( const U8* _data, U32 _blockSize );
	void	ResetPacket();

	friend void	SerialSendPacketsTask( void* _param );
	friend void	SerialReceivePacketsTask( void* _param );
};
/*
// The Serial Receiver class is used as a buffer for serial packets sent by the PC
class Serial_Receiver : public Serial_Base {
public:

};

// The Serial Transmitter class is used to send serial packets to the PC
class Serial_Transmitter : public Serial_Base {
public:


	void	begin( int ) {}
	void	println( const char* a = NULL ) {}
	void	printf( const char* a, ... ) {}
};
*/
#ifdef NO_GLOBAL_SERIAL
extern CentralSerial	Serial;
#endif