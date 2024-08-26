#include "../Global.h"
#include "AudioTransformers.h"

void	TransformMono2Stereo::GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount ) {


#if 0	// I think that's the solution to a proper wave generation => Use precomputed sine tables... :/
static U32	s_sampleIndex = 0;
for ( U32 i=0; i < _samplesCount; i++, _samples++ ) {
	S32	temp = FastSine( 743 * s_sampleIndex++ ) / 8;
	_samples->left = temp;
	_samples->right = temp;
}
return;

#elif 0
static S16*	s_wave = NULL;
static U32	s_sampleIndex = 0;
if ( s_wave == NULL ) {
	s_wave = new S16[32768];
	for ( U32 i=0; i < 32768; i++ ) {
//		s_wave[i] = 32767.5 * sin( 2*3.14159265358979f * i / 32768 );
		s_wave[i] = 4096 * sin( 2*3.14159265358979f * i / 32768 );
	}
}

for ( U32 i=0; i < _samplesCount; i++, _samples++ ) {
//S32	temp = S16( 4096 * sin( 2*3.14159265358979f * (1000.0f / _samplingRate) * s_sampleIndex++ ) );
S32	temp = s_wave[(743 * s_sampleIndex++) & 0x7FFF];
	_samples->left = temp;
	_samples->right = temp;
}
return;
#elif 0
// Simulate a 1KHz sine wave
static U32	s_sampleIndex = 0;
for ( U32 i=0; i < _samplesCount; i++, _samples++ ) {
S32	temp = S16( 4096 * sin( 2*3.14159265358979f * (1000.0f / _samplingRate) * s_sampleIndex++ ) );
	_samples->left = temp;
	_samples->right = temp;
}
return;
#endif


	m_source.GetSamples( _samplingRate, _samples, _samplesCount );

	// Convert mono source to stereo
	S16*	monoSample = (S16*) _samples + _samplesCount;
	Sample*	stereoSample = _samples + _samplesCount;
	for ( U32 sampleIndex=0; sampleIndex < _samplesCount; sampleIndex++ ) {
		monoSample--;
		stereoSample--;
		stereoSample->left = stereoSample->right = *monoSample;
	}
}
