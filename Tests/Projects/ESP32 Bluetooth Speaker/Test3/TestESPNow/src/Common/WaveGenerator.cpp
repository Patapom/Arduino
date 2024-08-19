#include "Global.h"
#include "WaveGenerator.h"

U32	WaveGenerator::GetSamples( Sample* _samples, U32 _samplesCount ) {
	if ( m_channelsCount == MONO ) {
		S16*	samples = (S16*) _samples;
		for ( U32 sampleIndex; sampleIndex < _samplesCount; sampleIndex++, samples++ ) {
			*samples = S16( 32767.5f * clamp( m_amplitudeLeft * float(sin( 2*float(PI) * sampleIndex / m_samplingRate * m_frequencyLeft + m_phaseLeft )), -1.0f, 1.0f ) );
		}
	} else {
		for ( U32 sampleIndex; sampleIndex < _samplesCount; sampleIndex++, _samples++ ) {
			_samples->left = S16( 32767.5f * clamp( m_amplitudeLeft * float(sin( 2*float(PI) * sampleIndex / m_samplingRate * m_frequencyLeft + m_phaseLeft )), -1.0f, 1.0f ) );
			_samples->right = S16( 32767.5f * clamp( m_amplitudeRight * float(sin( 2*float(PI) * sampleIndex / m_samplingRate * m_frequencyRight + m_phaseRight )), -1.0f, 1.0f ) );
		}
	}
	m_sampleIndex += _samplesCount;

	return _samplesCount;
}
