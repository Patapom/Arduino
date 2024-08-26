#pragma once

#include "../AudioBuffer.h"

// Transformer from mono => stereo, same sampling rate
class TransformMono2Stereo : public ISampleSource {
	ISampleSource&	m_source;

public:
	TransformMono2Stereo( ISampleSource& _source ) : m_source( _source ) {}

    virtual U32			GetSamplingRate() const override { return m_source.GetSamplingRate(); }
	virtual CHANNELS	GetChannelsCount() const override { return STEREO; }
	virtual void		GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount ) override;
};
