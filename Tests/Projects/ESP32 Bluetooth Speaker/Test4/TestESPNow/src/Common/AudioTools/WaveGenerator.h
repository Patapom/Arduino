#pragma once

#include "../AudioBuffer.h"

class WaveGenerator : public ISampleSource {
public:
	CHANNELS	m_channelsCount;

	float		m_amplitudeLeft = 32767.5f;
	float		m_frequencyLeft = 1000.0f;
	float		m_phaseLeft = 0;

	float		m_amplitudeRight = 32767.5f;
	float		m_frequencyRight = 1000.0f;
	float		m_phaseRight = 0;

private:
	static S16*	ms_sine;
	U64			m_time = 0;

public:
	void	SetChannelsCount( CHANNELS _channelsCount ) { m_channelsCount = _channelsCount; }

	void	SetWaveLeft( float _frequency, float _amplitude=1, float _phase=0 );
	void	SetWaveRight( float _frequency, float _amplitude=1, float _phase=0 );

	// ISampleSource Implementation
	virtual U32			GetSamplingRate() const override { return ~0UL; }	// Doesn't mean anything since we're generating a continuous signal...
	virtual CHANNELS	GetChannelsCount() const override { return m_channelsCount; }
	virtual void		GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount );
};
