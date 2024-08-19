#include "Global.h"
#include "AudioTransformers.h"

void	AudioTransformer::Init( AudioBuffer& _source ) {
	m_source = &_source;
}

//bool	TransformMono2Stereo::GetSamples( AudioBuffer& _audioBuffer, Sample* _samples, U32 _requestedSamplesCount ) {
U32	TransformMono2Stereo::GetSamples( Sample* _samples, U32 _requestedSamplesCount ) {
	S16*	buffer = (S16*) m_source->m_buffer;
	U32		bufferSize = m_source->m_bufferSize << 1;	// Mono samples count = 2 * Stereo samples count

	U32	totalLoadedSamplesCount = 0;
	while ( _requestedSamplesCount > 0 ) {
		// Feed as many samples as are available in the buffer
		U32	preloadedSamplesCount = m_source->m_sampleIndexWrite << 1;	// Mono samples count = 2 * Stereo samples count
		U32	availableSamplesCount = preloadedSamplesCount - m_sampleIndex;
		if ( availableSamplesCount == 0 ) {
//Serial.println( "Starved!" );
			if ( !m_source->m_autoPreLoad || !m_source->UpdateBuffer( _requestedSamplesCount ) ) {
				return 0;	// Starved!
			}
			preloadedSamplesCount = m_source->m_sampleIndexWrite << 1;	// Mono samples count = 2 * Stereo samples count
			availableSamplesCount = preloadedSamplesCount - m_sampleIndex;
			ERROR( availableSamplesCount == 0, "Still starved after UpdateBuffer()?" );	// The buffer should have been filled with new data!
		}

		U32	samplesCount = min( availableSamplesCount, _requestedSamplesCount );
		U32	loadedSamplesCount = 0;
		U32	bufferSampleIndex = m_sampleIndex % bufferSize;	// Constrain within the buffer
		U32	samplesCountToEnd = bufferSize - bufferSampleIndex;	// How many can we load until we reach the end of the buffer?
		if ( samplesCountToEnd < samplesCount ) {
			// Copy to the end of the buffer then loop around
			S16*	source = buffer + bufferSampleIndex;
			for ( U32 i=0; i < samplesCountToEnd; i++, _samples++ ) {
				S16	value = *source++;
				_samples->left = value;
				_samples->right = value;
			}
			loadedSamplesCount += samplesCountToEnd;
			samplesCount -= samplesCountToEnd;

			// Loop back
			buffer = (S16*) m_source->m_buffer;
			bufferSampleIndex = 0;
		}

		// Read the rest
		S16*	source = buffer + bufferSampleIndex;
		for ( U32 i=0; i < samplesCount; i++, _samples++ ) {
			S16	value = *source++;
			_samples->left = value;
			_samples->right = value;
		}
		loadedSamplesCount += samplesCount;

		// Advance the sample cursor and decrease the amount of requested samples by the amount we just loaded
		m_sampleIndex += loadedSamplesCount;
		_requestedSamplesCount -= loadedSamplesCount;
		totalLoadedSamplesCount += loadedSamplesCount;

		// Also update the audio buffer's read index & optionally pre-load next samples
		// (it also works if we're "in the middle of a stereo sample" since we don't want UpdateBuffer() to overwrite a sample we're still playing)
		m_source->m_sampleIndexRead = m_sampleIndex >> 1;
		if ( m_source->m_autoPreLoad ) {
			if ( m_source->UpdateBuffer( loadedSamplesCount >> 1 ) ) {	// Pre-load next samples to replace the one we just used...
//Serial.printf( "Updated buffer with %d new samples\n", loadedSamplesCount >> 1 );
			}
		}
	}

	return totalLoadedSamplesCount;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
void	TransformInterpolateMono2Stereo::SetTargetSamplingRate( U32 _targetSamplingRate ) {
	m_targetSamplingRate = _targetSamplingRate;
//	m_sampleInc = (m_source->GetSamplingRate() << 8) / m_targetSamplingRate;
	m_sampleInc = float(m_source->GetSamplingRate()) / m_targetSamplingRate;
}

//bool	TransformInterpolateMono2Stereo::GetSamples( AudioBuffer& _audioBuffer, Sample* _samples, U32 _requestedSamplesCount ) {
U32	TransformInterpolateMono2Stereo::GetSamples( Sample* _samples, U32 _requestedSamplesCount ) {
	S16*	buffer = (S16*) m_source->m_buffer;
	U32		bufferSize = m_source->m_bufferSize << 1;	// Mono samples count = 2 * Stereo samples count

	U32	totalLoadedSamplesCount = 0;
	while ( _requestedSamplesCount > 0 ) {
		// Feed as many samples as are available in the buffer
		U32	preloadedSamplesCount = m_source->m_sampleIndexWrite << 1;	// Mono samples count = 2 * Stereo samples count

		U32	sourceSampleIndexStart = U32( floor( m_sampleIndex ) ) >> 1;	// Stereo samples are half the amount of mono samples
		U32	samplesCount = 0;
		while ( m_sampleIndex < preloadedSamplesCount && samplesCount < _requestedSamplesCount ) {
			U32		sampleIndex0 = U32( floor( m_sampleIndex ) );
			float	t = m_sampleIndex - sampleIndex0;
					sampleIndex0 %= bufferSize;
			U32		sampleIndex1 = (sampleIndex0 + 1) % bufferSize;

			// Interpolate a new sample value
			S32		value0 = buffer[sampleIndex0];
			S32		value1 = buffer[sampleIndex1];
			S16		value = S16( value0 + t * (value1 - value0) );
			_samples->left = value;
			_samples->right = value;
			_samples++;
			samplesCount++;

			m_sampleIndex += m_sampleInc;
		}
		_requestedSamplesCount -= samplesCount;
		totalLoadedSamplesCount += samplesCount;

		// Set the audio buffer's actual (stereo) sample index
		U32	sourceSampleIndexEnd = U32( floor( m_sampleIndex ) ) >> 1;	// Stereo samples are half the amount of mono samples
		m_source->m_sampleIndexRead = sourceSampleIndexEnd;

		// Either load as many samples as we just read, or read the entire buffer if we're starved
		U32	usedSamplesCount = sourceSampleIndexEnd - sourceSampleIndexStart;	// Amount of actual samples we used from the buffer
//Serial.printf( "Fed %d samples, Update %d samples - preloaded %d\n", samplesCount, usedSamplesCount, preloadedSamplesCount );
		if ( !m_source->m_autoPreLoad || m_source->UpdateBuffer( usedSamplesCount > 0 ? usedSamplesCount : m_source->m_bufferSize ) == 0 ) {
//Serial.println( "Starved!" );
			return 0;	// Starved!
		}
	}

	return totalLoadedSamplesCount;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
/*TransformInterpolateStereo::TransformInterpolateStereo( U32 _sourceSamplingRate, U32 _targetSamplingRate )
	: m_sourceSamplingRate( _sourceSamplingRate )
	, m_targetSamplingRate( _targetSamplingRate )
{
}

void	TransformInterpolateStereo::Transform( const Sample* _sourceSamples, Sample*& _targetSamples, U32 _samplesCount ) {
	const Sample*	current = _sourceSamples;
	const Sample* 	next = _sourceSamples+1;	// <= Problem! Could sample beyond buffer!
	U32				index = 0;					// <= Problem! We should keep a "float" index in the source stream

	for ( U32 sampleIndex=0; sampleIndex < _samplesCount; sampleIndex++, _targetSamples++ ) {

		S16	value = *sourceSample++;
		_targetSamples->left = value;
		_targetSamples->right = value;
	}
}
*/
