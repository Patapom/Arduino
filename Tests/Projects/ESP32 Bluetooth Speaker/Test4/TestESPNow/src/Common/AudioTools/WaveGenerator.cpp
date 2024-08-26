#include "../Global.h"
#include "WaveGenerator.h"

S16*	WaveGenerator::ms_sine = NULL;

void	WaveGenerator::SetWaveLeft( float _frequency, float _amplitude, float _phase ) {
	m_frequencyLeft = _frequency;
	m_amplitudeLeft = _amplitude * 32767.5f;
	m_phaseLeft = _phase;
}

void	WaveGenerator::SetWaveRight( float _frequency, float _amplitude, float _phase ) {
	m_frequencyRight = _frequency;
	m_amplitudeRight = _amplitude * 32767.5f;
	m_phaseRight = _phase;
}

// So apparently it's not possible to generate 2 sine waves at 44100Hz ???
#if 1
void	WaveGenerator::GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount ) {
	static U32	time = 0;
	S16*	samples = (S16*) _samples;
	for ( U32 sampleIndex; sampleIndex < _samplesCount; sampleIndex++ ) {
		*samples++ = S16( 4096 * sin( 2*PI * (1000.0 / _samplingRate) * time++ ) );	// THIS IS FUCKING WORKING!!!!!
	}
}
#elif 1
void	WaveGenerator::GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount ) {
	if ( ms_sine == NULL ) {
		ms_sine = new S16[32768];
		for ( U32 i=0; i < 32768; i++ ) {
			ms_sine[i] = 32767 * sin( 2*3.14159265358979f * i / 32768 );
		}
	}

	U64		time = m_time;
	m_time += _samplesCount;

	U64	timeFactorLeft = m_frequencyLeft * 32768 / _samplingRate;
	U64	timeFactorRight = m_frequencyRight * 32768 / _samplingRate;
	S32	amplitudeLeft = 8;//32767 / m_amplitudeLeft;
	S32	amplitudeRight = 8;//32767 / m_amplitudeRight;

	if ( m_channelsCount == MONO ) {
		S16*	samples = (S16*) _samples;
		for ( U32 sampleIndex; sampleIndex < _samplesCount; sampleIndex++ ) {
//*samples++ = S16( 4096 * sin( 2*PI * (1000.0 / _samplingRate) * time++ ) );	// THIS IS FUCKING WORKING!!!!!
//			*samples++ = (amplitudeLeft * ms_sine[(timeFactorLeft * time++) & 0x7FFFULL] ) / 16384;
//			*samples++ = ms_sine[(timeFactorLeft * time++) & 0x7FFFULL] / 8;	// FUCKING WORKING!!
			*samples++ = ms_sine[(timeFactorLeft * time++) & 0x7FFFULL] / amplitudeLeft;
		}
	} else {
		for ( U32 sampleIndex; sampleIndex < _samplesCount; sampleIndex++, _samples++, time++ ) {
//			_samples->left = (amplitudeLeft * S32( ms_sine[(timeFactorLeft * time) & 0x7FFF] )) / 32767;
_samples->left = ms_sine[(timeFactorLeft * time) & 0x7FFFULL] / amplitudeLeft;
_samples->left = S16( 4096 * sin( 2*PI * (1000.0 / _samplingRate) * time ) );

//_samples->right = ms_sine[(timeFactorRight * time) & 0x7FFFULL] / amplitudeRight;
//_samples->left = (amplitudeLeft * S32( ms_sine[(timeFactorLeft * time++) & 0x7FFFUL] )) / 32767;
//_samples->left = ms_sine[(timeFactorLeft * time++) & 0x7FFFUL];
			_samples->right = 0;// (amplitudeRight * S32( ms_sine[(timeFactorRight * time) & 0x7FFF] )) / 32767;
//_samples->left = (time & 0x10) ? 4096 : -4096;
//_samples->right = (time & 0x20) ? 4096 : -4096;
		}
	}
}
#elif 0
void	WaveGenerator::GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount ) {
	float	deltaTimeLeft = 2*PI * m_frequencyLeft / _samplingRate;
	float	deltaTimeRight = 2*PI * m_frequencyRight / _samplingRate;
//	float	timeRight = 2*PI * m_frequencyRight * (m_time / 1000000.0f) + m_phaseRight;

//	float	dT = 1.0f / _samplingRate;
//	float	timeFactorLeft = m_frequencyLeft / 1000000.0f;
//	float	timeFactorRight = m_frequencyLeft / 1000000.0f;

	if ( m_channelsCount == MONO ) {
		S16*	samples = (S16*) _samples;
		float	timeLeft = m_timeLeft;
		for ( U32 sampleIndex; sampleIndex < _samplesCount; sampleIndex++, samples++ ) {
//			float	time = (m_time + sampleIndex * 1000000ULL / _samplingRate) * (m_frequencyLeft / 1000000.0f);
//			*samples = S16( 32767.5f * clamp( m_amplitudeLeft * sin( 2*float(PI) * time + m_phaseLeft ), -1.0f, 1.0f ) );
*samples = S16( m_amplitudeLeft * sin( timeLeft ) );	timeLeft += deltaTimeLeft;
		}
	} else {
		float	timeLeft = m_timeLeft;
		float	timeRight = m_timeRight;
		for ( U32 sampleIndex; sampleIndex < _samplesCount; sampleIndex++, _samples++ ) {
//			U64		time =  + sampleIndex * 1000000ULL / _samplingRate;
//			float	timeLeft = time * (m_frequencyLeft / 1000000.0f);
//			float	timeRight = time * (m_frequencyRight / 1000000.0f);
//			_samples->left = S16( 32767.5f * clamp( m_amplitudeLeft * sin( 2*float(PI) * timeLeft + m_phaseLeft ), -1.0f, 1.0f ) );
//			_samples->right = S16( 32767.5f * clamp( m_amplitudeRight * sin( 2*float(PI) * timeRight + m_phaseRight ), -1.0f, 1.0f ) );

_samples->left = S16( m_amplitudeLeft * sin( timeLeft ) );	timeLeft += deltaTimeLeft;
_samples->right = S16( m_amplitudeRight * sin( timeRight ) );	timeRight += deltaTimeRight;
		}
	}

	m_timeLeft += _samplesCount * deltaTimeLeft;
	m_timeRight += _samplesCount * deltaTimeRight;
}
#else
void	WaveGenerator::GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount ) {
	float	timeFactorLeft = 2*PI * m_frequencyLeft / _samplingRate;
	float	timeFactorRight = 2*PI * m_frequencyRight / _samplingRate;

	U64		time = m_time;
	m_time += _samplesCount;

	if ( m_channelsCount == MONO ) {
		S16*	samples = (S16*) _samples;
		for ( U32 sampleIndex; sampleIndex < _samplesCount; sampleIndex++ ) {
			*samples++ = S16( m_amplitudeLeft * sin( timeFactorLeft * (time++) + m_phaseLeft ) );
		}
	} else {
//		U64	time0 = m_time, ampLeft = m_amplitudeLeft, phaseLeft = m_phaseLeft, ampRight = m_amplitudeRight, phaseRight = m_phaseRight;	// WHY?! OH WHY DO WE HAVE TO COPY VARIABLES LOCALLY TO MAKE THIS CODE A THOUSAND TIMES FASTER??
//		U64	time;
//		for ( U32 sampleIndex; sampleIndex < _samplesCount; sampleIndex++, _samples++ ) {
//			time = time0 + sampleIndex * 1000000ULL / _samplingRate;
//			_samples->left = S16( ampLeft * sin( timeFactorLeft * time + phaseLeft ) );
//			_samples->right = S16( ampRight * sin( timeFactorRight * time + phaseRight )  );
//		}

for ( U32 i=0; i < _samplesCount; i++, _samples++, time++ ) {
// Even this trips the watchdog!?
//	_samples->left = S16( 4095 * sin( timeFactorLeft * time ) );
//	_samples->right = S16( 4095 * sin( timeFactorRight * time ) );

	_samples->left = S16( m_amplitudeLeft * sin( timeFactorLeft * time + m_phaseLeft ) );
//// WHAT THE FUCK? Generating 2 wave forms takes too much time?! The resulting sound trips the watchdog!
//	_samples->right = S16( m_amplitudeRight * sin( timeFactorRight * time + m_phaseRight ) );
	_samples->right = 0;//_samples->left;
}
	}
}
#endif