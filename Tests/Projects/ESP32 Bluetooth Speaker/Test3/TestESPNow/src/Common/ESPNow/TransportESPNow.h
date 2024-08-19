// Inspired by Atomic14
#pragma once

#include <WiFi.h>
#include <esp_now.h>

#include "../AudioBuffer.h"

class TransportESPNow_Base {
public:
	static const U32	SAMPLES_PER_PACKET = 61;	// An ESP-Now packet is 250 bytes, we use 6 bytes of header so we can only pack 244 / 4 bytes per sample = 61 samples in a single packet

public:
	U16	m_header = 0x1234;	// Default packet header

public:

	TransportESPNow_Base();
	virtual ~TransportESPNow_Base();

	// Set the packet header that is sent by the sender/expected by the receiver
	void	SetHeader( U16 _header ) { m_header = _header; }

protected:
	void	Init( U8 _WiFiChannel );
};

class TransportESPNow_Receiver : public TransportESPNow_Base, public ISampleSource {
  friend void	ReceiveCallback( const U8* _MACAddress, const U8* _data, int _dataLength );
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

	// Audio buffer for samples we are receiving
	// This is a circular buffer that will be filled with the samples received by the device
	// It's up to the player to make sure it catches up to the received samples...
	Sample*		m_buffer = NULL;
	U32			m_bufferSize = 0;			// How many samples in the buffer?
	U32			m_samplingRate = 44100;		// 44100Hz frequency
	CHANNELS	m_channelsCount = STEREO;	// Stero
	U32			m_sampleIndexWrite = 0;		// Index of the next sample we will write to when we receive a new audio packet
	U32			m_sampleIndexRead = 0;		// Index of the next sample to be read via the ISampleSource interface

	OnPacketsReceivedCallback*	m_onPacketsReceivedCallback = NULL;

public:	// Status
	RECEIVED_PACKET_STATUS	m_lastReceivedPacketStatus = BUFFER_EMPTY;
	U32			m_receivedPacketsCount = 0;
	U32			m_lostPacketsCount = 0;
	U32			m_lastReceivedPacketID = 0;

public:
	TransportESPNow_Receiver();
	~TransportESPNow_Receiver();

	void	Init( U8 _WiFiChannel, U8 _receiverIDMask, U32 _bufferSize );
	void	SetOnPacketReceivedCallback( OnPacketsReceivedCallback* _callback ) { m_onPacketsReceivedCallback = _callback; }

	void	SetSamplingRate( U32 _samplingRate ) { m_samplingRate = _samplingRate; }
	void	SetChannelsCount( CHANNELS _channelsCount ) { m_channelsCount = _channelsCount; }

	// ISampleSource immplementation
	virtual U32			GetSamplingRate() const override { return m_samplingRate; }
	virtual CHANNELS	GetChannelsCount() const override { return m_channelsCount; }
	virtual U32	 		GetSamples( Sample* _samples, U32 _samplesCount ) override;

private:
	void	Receive( const U8* _senderMACAddress, const U8* _payload, U32 _payloadSize );
};

class TransportESPNow_Transmitter : public TransportESPNow_Base {
public:
	U8	m_buffer[ESP_NOW_MAX_DATA_LEN];
	U32	m_sentPacketsCount = 0;

public:

	void	Init( U8 _WiFiChannel );

	// Sends a 250 bytes packet containing 61 samples
	//	_samplesSource, the source to sample from. NOTE: The source *MUST* have at least 61 samples available to fill the entire packet, otherwise an exception is thrown!
	//	_packetID, the ID of the packet. NOTE: The ID is 24-bits only, MSB will be ignored *BUT* if your packet IDs are contiguous then looping 24-bits IDs can be detected and a 32-bits ID can be reconstructed.
	//	_receiverMaskID, the ID of the targeted receivers, as a mask (i.e. receivers have an ID in [1,8], each bit targets a specific receiver, e.g. 0xFF targets all receivers). NOTE: The central has the special ID 0
	void	SendPacket( ISampleSource& _sampleSource, U32 _packetID, U8 _receiverMaskID=0xFF );

private:
	void	Send();
};
