// Inspired by Atomic14
#pragma once
#include <WiFi.h>
#include <esp_now.h>

#include "../AudioBuffer.h"

//#define ENABLE_PACKET_DOUBLING	// Enable this to send each packet twice, doubling the chance of receiving them on the other end...

class TransportESPNow_Base {
public:
	static const U32	SAMPLES_PER_PACKET = 61;	// An ESP-Now packet is 250 bytes long, we use 6 bytes of header so we can only pack 244 / 4 bytes per sample = 61 samples in a single packet

public:
	U16		m_header = 0x1234;	// Default packet header

	U32		m_samplingRate = 0;	// The sampling rate we're expecting this transport pipeline to function at

public:

	virtual ~TransportESPNow_Base() {}

	// Must be called prior any usage of the TransportESPNow classes, to initialize the WiFi connection properly
	static void	ConfigureWiFi( U8 _WiFiChannel );

	// Set the packet header that is sent by the sender/expected by the receiver
	void		SetHeader( U16 _header ) { m_header = _header; }

	// Scans for WiFi networks using the specified channel, return NULL if unused or SSID of the network using the channel
	static const char*	CheckWiFiChannelUnused( U8 _channel );

	// Scans for all available WiFi networks
	static U32	DumpWiFiScan( bool _deleteScanOnExit=true );	// Returns how many networks were found

	// Scans all available WiFi networks and marks the channels they're using
	// We should always try and operate on free channels otherwise a lot of interference can occur!
	static U32	ScanWifiChannels( U8 _channels[11], bool _dump=false );	// Returns how many networks were found

protected:
	void	Init( U32 _samplingRate );
};

// The ESP-Now Receiver class is a sample source where samples are received by radio
class TransportESPNow_Receiver : public TransportESPNow_Base, public AudioBuffer {
public:
	enum RECEIVED_PACKET_STATUS {
		BUFFER_EMPTY = -1,		// The buffer is empty (no packet received yet)
		OK,						// Received okay
		NOT_FOR_US,				// Okay but not for us (receiver mask IDs don't overlap)
		INVALID_HEADER,			// Bad header
		INVALID_PAYLOAD_SIZE,	// Unexpected size for payload (expecting 61 samples)
	};

	// A callback that will be called every time we receive a new packet
	// _formerPacketID indicates the last packet ID we had received *BEFORE* we received this new packet (whose ID you can get from this class's field m_lastReceivedPacketID), it can help determine how many packets were lost since last callback...
	typedef void	OnPacketsReceivedCallback( U32 _formerPacketID );

public:

	// The mask to compare to received mask IDs to accept or discard received packets
	U8			m_receiverMaskID = 0xFF;	// By default, accept all packets
	CHANNELS	m_channelsCount = STEREO;	// Stereo by default

	OnPacketsReceivedCallback*	m_onPacketsReceivedCallback = NULL;

	bool		m_blockPackets = false;

public:	// Status
	RECEIVED_PACKET_STATUS	m_lastReceivedPacketStatus = BUFFER_EMPTY;
	U32						m_receivedPacketsCount = 0;
	U32						m_lostPacketsCount = 0;
	U32						m_lastReceivedPacketID = 0;

public:
	TransportESPNow_Receiver( const ITimeReference& _time );
	~TransportESPNow_Receiver();

	//	_samplingRate, the sampling rate we're expecting this receiver to function at
	bool	Init( U8 _receiverIDMask, U32 _samplingRate, CHANNELS _channelsCount, float _preLoadDelay );
	void	SetOnPacketReceivedCallback( OnPacketsReceivedCallback* _callback ) { m_onPacketsReceivedCallback = _callback; }

	// ISampleSource immplementation
	virtual U32			GetSamplingRate() const override { return m_samplingRate; }
	virtual CHANNELS	GetChannelsCount() const override { return m_channelsCount; }

	// Blocks packet to simulate packets loss (for debugging purpose)
	void	BlockPackets( bool _blockPackets ) { m_blockPackets = _blockPackets; }

private:
	void	Receive( const U8* _senderMACAddress, const U8* _payload, U32 _payloadSize );

  	friend void	ReceiveCallback( const U8* _MACAddress, const U8* _data, int _dataLength );
};

// The ESP-Now Transmitter class is only a helper to send audio packets whenever enough samples are available from the source
class TransportESPNow_Transmitter : public TransportESPNow_Base {
private:
	const ITimeReference*	m_time = NULL;
	ISampleSource*			m_sampleSource = NULL;

	// Temporary buffer to store the packet payload
	U8		m_buffer[ESP_NOW_MAX_DATA_LEN];

	U8		m_receiverMaskID = 0xFF;	// Target all devices

public:

	bool	m_blockPackets = false;		// Use this to block the sending of packets (e.g. when no sound is present)
	U32		m_sentPacketsCount = 0;

public:

	TransportESPNow_Transmitter( const ITimeReference& _time ) : m_time( &_time ) {}

	void	Init( U32 _samplingRate );

	// Starts an auto-send task to send packets at the required sampling rate
	void	StartAutoSendTask( U8 _taskPriority, ISampleSource& _sampleSource, U8 _receiverMaskID );

	// Sends a 250 bytes packet containing 61 samples
	//	_samplesSource, the source to sample from. NOTE: The source *MUST* have at least 61 samples available to fill the entire packet, otherwise an exception is thrown!
	//	_packetID, the ID of the packet. NOTE: The ID is 24-bits only, MSB will be ignored *BUT* if your packet IDs are contiguous then looping 24-bits IDs can be detected and a 32-bits ID can be reconstructed.
	//	_receiverMaskID, the ID of the targeted receivers, as a mask (i.e. receivers have an ID in [1,8], each bit targets a specific receiver, e.g. 0xFF targets all receivers). NOTE: The central has the special ID 0
	void	SendPacket( ISampleSource& _sampleSource, U32 _packetID, U8 _receiverMaskID );

	// Sends a raw packet of 250 bytes
	void	SendRawPacket( const U8 _packet[ESP_NOW_MAX_DATA_LEN] );

private:
	void	Send();

	friend void	SendPacketsTask( void* _param );
};
