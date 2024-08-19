#pragma once

#include "driver/i2s.h"
#include "../AudioBuffer.h"

// I2S Output sampler to drive the MAX98357A
//
class I2SOutput {
	friend void	I2SWriterTask( void* _param );

public:

	static const U32	NUM_SAMPLES_TO_SEND = 512;	// Number of samples to try and send at once
//	static const U32	NUM_SAMPLES_TO_SEND = 61;	// Number of samples to try and send at once

private:
	TaskHandle_t	m_I2STaskHandle;	// I2S writer task
	QueueHandle_t	m_I2SQueue;			// I2S writer queue
	i2s_port_t		m_I2SPort;			// I2S port
	ISampleSource*	m_sampleSource;		// The source of samples to play

	// Volume settings
	float			m_volume = 1.0;
	S32				m_volumeInt = 65535;

public:
	void	Start( i2s_port_t _I2SPort, const i2s_pin_config_t& _pins, ISampleSource& _sampleSource );

	float	GetVolume() const { return m_volume; }
	void	SetVolume( float _volume ) { m_volume = saturate( _volume ); m_volumeInt = (S32) (m_volume * 65535); }

	// Directly write samples to the I2S device
	// Returns the amount of samples actually written
	U32		WriteSamples( Sample* _samples, U32 _samplesCount );

private:
	U32		GetSamples( Sample* _samples, U32 _requestedSamplesCount );
};

