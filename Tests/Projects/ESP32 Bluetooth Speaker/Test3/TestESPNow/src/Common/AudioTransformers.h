#pragma once

#include "AudioBuffer.h"

// Abstract audio transformer class
class AudioTransformer : public ISampleSource {
protected:
	AudioBuffer*	m_source = NULL;

public:
	void	Init( AudioBuffer& _source );

	// ISampleSource Implementation
    virtual U32			GetSamplingRate() const = 0;
	virtual CHANNELS	GetChannelsCount() const = 0;
    virtual U32			GetSamples( Sample* _samples, U32 _samplesCount ) = 0;	// This should fill the samples buffer with the specified number of samples
};

// Transformer from mono => stereo, same sampling rate
class TransformMono2Stereo : public AudioTransformer {
	U32	m_sampleIndex = 0;	// Index of the mono sample we're reading (stereo sample index is given by m_sampleIndex >> 1)

public:
    virtual U32			GetSamplingRate() const override { return m_source->GetSamplingRate(); }
	virtual CHANNELS	GetChannelsCount() const override { return STEREO; }
    virtual U32			GetSamples( Sample* _samples, U32 _samplesCount ) override;
//	virtual void		GetSamples( AudioBuffer& _audioBuffer, Sample* _samples, U32 _requestedSamplesCount );
};

// Transformer from mono => stereo, different sampling rate
class TransformInterpolateMono2Stereo : public AudioTransformer {
	U32		m_targetSamplingRate;
	float	m_sampleIndex = 0.0f;	// Index of the mono sample we're reading (stereo sample index is given by U32( floor( m_sampleIndex ) ) >> 1)
	float	m_sampleInc = 1.0;		// Increments to use to achieve the target sampling rate
public:
	void			SetTargetSamplingRate( U32 _targetSamplingRate );

    virtual U32			GetSamplingRate() const override { return m_targetSamplingRate; }
	virtual CHANNELS	GetChannelsCount() const override { return STEREO; }
    virtual U32			GetSamples( Sample* _samples, U32 _samplesCount ) override;
//	virtual bool	GetSamples( AudioBuffer& _audioBuffer, Sample* _samples, U32 _requestedSamplesCount );
};

/*
// Transformer using different sampling rate (stereo)
class TransformInterpolateStereo : public ITransformer {
	U32	m_sourceSamplingRate;
	U32	m_targetSamplingRate;
public:
	TransformInterpolateStereo( U32 _sourceSamplingRate, U32 _targetSamplingRate );
//	virtual void	Transform( const Sample* _sourceSamples, Sample*& _targetSamples, U32 _samplesCount ) override;
	virtual bool	GetSamples( AudioBuffer& _audioBuffer, Sample* _samples, U32 _requestedSamplesCount );
};
*/