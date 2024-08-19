// Inspired by Atomic14
#pragma once

#include <WiFi.h>
#include <esp_now.h>

#include "../AudioBuffer.h"

class TransportESPNow_Base {
public:
	static const U32	SAMPLES_PER_PACKET = 61;	// An ESP6Now packet is 250 bytes, we use 4 bytes of header so we can only pack 246 / 4 bytes per sample = 61 sample in a single packet

public:
	U32	m_headerSize = 0;

public:

	TransportESPNow_Base();
	virtual ~TransportESPNow_Base();

	// Set the packet header that is sent by the sender/expected by the receiver
	virtual void	SetHeader( U32 _headerSize, const U8* _header ) = 0;

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

protected:
	// Header we need to compare received payload to
	U8*		m_header = NULL;

	// The mask to compare to received mask IDs to accept or discard packets
	U8		m_receiverMaskID = 0xFF;	// By default, accept all packets

	// Audio buffer for samples we are receiving
	// This is a circular buffer that will be filled with the samples received by the device
	// It's up to the player to make sure it catches up to the received samples...
	Sample*	m_buffer = NULL;
	U32		m_bufferSize = 0;
	U32		m_samplingRate = 44100;	// 44100Hz frequency
	U32		m_sampleIndexWrite = 0;	// Index of the next sample we will write to when we receive a new audio packet
	U32		m_sampleIndexRead = 0;	// Index of the next sample to be read via the ISampleSource interface

public:
	RECEIVED_PACKET_STATUS	m_lastReceivedPacketStatus = BUFFER_EMPTY;
	U32		m_receivedPacketsCount = 0;
	U32		m_lostPacketsCount = 0;
	U8		m_lastReceivedPacketID = 0xFF;

public:
	TransportESPNow_Receiver();
	~TransportESPNow_Receiver();

	void			Init( U8 _WiFiChannel, U8 _receiverIDMask, U32 _bufferSize );
	virtual void	SetHeader( U32 _headerSize, const U8* _header ) override;

	// ISampleSource immplementation
	virtual U32		GetSamplingRate() override { return m_samplingRate; }
	virtual void	SetSamplingRate( U32 _samplingRate ) { m_samplingRate = _samplingRate; }
	virtual void 	GetSamples( Sample* _samples, U32 _samplesCount ) override;

private:
	void	Receive( const U8* _senderMACAddress, const U8* _payload, U32 _payloadSize );
};

class TransportESPNow_Transmitter : public TransportESPNow_Base {
public:
	U8* m_buffer = NULL;
	U8	m_payloadSize = 0;

public:
	TransportESPNow_Transmitter();
	~TransportESPNow_Transmitter();

	void			Init( U8 _WiFiChannel );
	virtual void	SetHeader( U32 _headerSize, const U8* _header ) override;

	// Writes a new sample to the buffer, sends the packet once enough samples have been written to fill an ESP-Now payloed (i.e. 250 bytes)
//	void	WriteSample( const Sample& _sample );
//	void	WriteSamples( ISampleSource& _sampleSource, U32 _samplesCount );
	void	SendPacket( ISampleSource& _sampleSource, U8 _packetID, U8 _receiverMaskID=0xFF );

	void	Flush();

private:
	void	Send();
};
