#pragma once

#if 0

#include <WiFi.h>
#include <esp_now.h>

#include "../AudioBuffer.h"

// Inspired by Atomic14
class AudioBufferESPNow : public ISampleSource {
  friend void	ReceiveCallback( const U8* _MACAddress, const U8* _data, int _dataLength );
public:

protected:
	// Audio buffer for samples we need to send
	U8* m_bufferSend = NULL;
	U32	m_bufferSendSize = 0;
	U32	m_sampleSendIndex = 0;
	U32 m_headerSize = 0;

	// Audio buffer for samples we are receiving
	// This is a circular buffer that will be filled with the samples received by the device
	// It's up to the player to make sure it catches up to the received samples...
	U8*	m_bufferReceive = NULL;
	U32	m_bufferReceiveSize = 0;
	U32	m_sampleReceiveIndex = 0;
	U32	m_samplingRate = 44100;		// 44100Hz frequency
	U32	m_playerSampleIndex = 0;	// Index of the next sample to be sent through the ISampleSource interface

public:
	AudioBufferESPNow( U32 _bufferReceiveSize, U32 _bufferSendSize = ESP_NOW_MAX_DATA_LEN );
	~AudioBufferESPNow();
	void	Init( U8 _WiFiChannel );

	// Set the packet header that is sent by the sender/expected by the receiver
	void	SetHeader( U32 _headerSize, const U8* _header );

	// Adds a new sample to the buffer, sends the packet once enough samples have been collected
	void	AddSample( const Sample& _sample );
	void	Flush();

	// ISampleSource immplementation
	virtual U32		GetSamplingRate() override { return m_samplingRate; }
	virtual void	SetSamplingRate( U32 _samplingRate ) { m_samplingRate = _samplingRate; }
	virtual void 	GetSamples( Sample* _samples, U32 _samplesCount ) override;

	// Accesses the buffer of received samples
	const U8*		GetBufferReceive() const 		{ return m_bufferReceive; }
	const U32		GetSampleReceiveIndex() const 	{ return m_sampleReceiveIndex; }

private:
	void	Send();
	void	Receive( const U8* _senderMACAddress, const U8* _payload, U32 _payloadSize );
};

#endif