#pragma once

#include "AudioBuffer.h"

class WaveGenerator : public ISampleSource {
public:
	CHANNELS	m_channelsCount;
	U32			m_samplingRate;
	U32			m_sampleIndex;

	float		m_amplitudeLeft = 1000;
	float		m_frequencyLeft = 1000;
	float		m_phaseLeft = 0;

	float		m_amplitudeRight = 1000;
	float		m_frequencyRight = 1000;
	float		m_phaseRight = 0;

public:
	void	SetChannelsCount( CHANNELS _channelsCount ) { m_channelsCount = _channelsCount; }
	void	SetSamplingRate( U32 _samplingRate ) { m_samplingRate = _samplingRate; }

	void	SetWaveLeft( float _frequency, float _amplitude=1, float _phase=0 ) { m_frequencyLeft = _frequency; m_amplitudeLeft = _amplitude; m_phaseLeft = _phase; }
	void	SetWaveRight( float _frequency, float _amplitude=1, float _phase=0 ) { m_frequencyRight = _frequency; m_amplitudeRight = _amplitude; m_phaseRight = _phase; }

	// ISampleSource Implementation
	virtual U32			GetSamplingRate() const override { return m_samplingRate; }
	virtual CHANNELS	GetChannelsCount() const override { return m_channelsCount; }
	virtual U32			GetSamples( Sample* _samples, U32 _samplesCount ) override;
};
