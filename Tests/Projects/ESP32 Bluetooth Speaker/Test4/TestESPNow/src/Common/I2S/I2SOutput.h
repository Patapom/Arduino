#pragma once

#include "driver/i2s.h"
#include "../AudioBuffer.h"

// I2S Output sampler to drive the MAX98357A
//
class I2SOutput {
public:

	static const U32	DMA_SIZE = 256;	// Size of the DMA buffer (and consequently, the amount of samples requested by the I2S TX events)

private:

	const ITimeReference*	m_time = NULL;

	TaskHandle_t	m_I2STaskHandle;	// I2S writer task
	QueueHandle_t	m_I2SQueue;			// I2S writer queue
	i2s_port_t		m_I2SPort;			// I2S port
	ISampleSource*	m_sampleSource;		// The source of samples to play
	U32 			m_samplingRate;		// The sampling rate at which we're playing
	float			m_sampleInterval;	// Delta time between 2 samples

	// Volume settings
	float			m_volume = 1.0;
	S32				m_volumeInt = 65535;

public:
	I2SOutput( const ITimeReference& _time ) : m_time( &_time ) {}

	void	Init( i2s_port_t _I2SPort, const i2s_pin_config_t& _pins, ISampleSource& _sampleSource, U32 _samplingRate );

	float	GetVolume() const { return m_volume; }
	void	SetVolume( float _volume ) { m_volume = saturate( _volume ); m_volumeInt = (S32) (m_volume * 65535); }

	U32		GetSamplingRate() const { return m_samplingRate; }

	// Directly write samples to the I2S device
	// Returns the amount of samples actually written
	U32		WriteSamples( Sample* _samples, U32 _samplesCount );

private:
	void	GetSamplesFromSource( Sample* _samples, U32 _requestedSamplesCount );

	friend void	I2SWriterTask( void* _param );
};

