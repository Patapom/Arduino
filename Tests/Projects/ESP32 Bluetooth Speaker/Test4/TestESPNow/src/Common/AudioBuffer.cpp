#include "Global.h"
#include "AudioBuffer.h"
#include <freertos/FreeRTOS.h>


// Gros problème de précision qqpart quand on atteint ~262s de fonctionnement: on tombe soudainement de 22 à 15KHz: (reproduit 2 fois au même endroit)
//260.261475 > Read = 5729131.50, 260.239 (Sampling Rate = 21950.13) | Write = 5740561, 260.760 (Sampling Rate = 21999.20/21738.26 Hz) | Updates Count = 85/s
//261.262451 > Read = 5751147.00, 261.237 (Sampling Rate = 22050.57) | Write = 5762577, 261.756 (Sampling Rate = 22051.86/21738.26 Hz) | Updates Count = 85/s
//262.263458 > Read = 5772877.50, 262.224 (Sampling Rate = 21912.55) | Write = 5784593, 262.759 (Sampling Rate = 21947.67/21738.26 Hz) | Updates Count = 85/s
// ======================== WTF?!!! ======================== 
//263.264465 > Read = 5792690.00, 263.222 (Sampling Rate = 16721.36) | Write = 5801745, 263.764 (Sampling Rate = 17004.11/16879.12 Hz) | Updates Count = 66/s
//264.265503 > Read = 5808932.50, 264.244 (Sampling Rate = 15607.47) | Write = 5816849, 264.751 (Sampling Rate = 15298.38/15088.91 Hz) | Updates Count = 59/s
//265.266479 > Read = 5824196.00, 265.242 (Sampling Rate = 15297.06) | Write = 5832209, 265.766 (Sampling Rate = 15153.57/15088.91 Hz) | Updates Count = 59/s
//266.267487 > Read = 5839401.00, 266.241 (Sampling Rate = 15318.82) | Write = 5847313, 266.757 (Sampling Rate = 15240.60/15088.91 Hz) | Updates Count = 59/s
//267.268494 > Read = 5854669.00, 267.239 (Sampling Rate = 15330.72) | Write = 5862673, 267.761 (Sampling Rate = 15287.82/15088.91 Hz) | Updates Count = 59/s



AudioBuffer::AudioBuffer( const ITimeReference& _time )
	: m_time( &_time )
{
#ifdef DEBUG_LOG
	memset( m_log, 0, 256*sizeof(U32) );
#endif
}
AudioBuffer::~AudioBuffer() {
	if ( m_buffer != NULL ) {
		free( m_buffer );
	}
}

bool	AudioBuffer::Init( U32 _bufferSize, U64 _sampleIndexWrite, U64 _preloadDelay_Micros ) {

	// Allocate pre-load buffer
	m_preloadDelay_Micros = _preloadDelay_Micros;
	m_bufferSize = _bufferSize;

	if ( m_buffer != NULL ) {
		free( m_buffer );	// Clear any existing buffer...
	}

	if ( GetChannelsCount() == STEREO ) {
		m_buffer = (Sample*) malloc( m_bufferSize * sizeof(Sample) );
		ERROR( m_buffer == NULL, "Failed to allocate pre-load buffer! (out of memory)" );
		memset( m_buffer, 0, m_bufferSize * sizeof(Sample) );
	} else {
		m_buffer = (Sample*) malloc( m_bufferSize * sizeof(S16) );
		ERROR( m_buffer == NULL, "Failed to allocate pre-load buffer! (out of memory)" );
		memset( m_buffer, 0, m_bufferSize * sizeof(S16) );
	}

	m_sampleIndexRead = 0;
	m_timeRead = 0;

	m_sampleIndexWrite = _sampleIndexWrite;
	m_timeWrite = m_preloadDelay_Micros;

//	m_semaphore = xSemaphoreCreateBinary();
//	xSemaphoreGive( m_semaphore );

	// Source: https://techoverflow.net/2023/01/06/esp32-critical-zone-example-using-freertos-platformio-on-esp32/
	// portENTER_CRITICAL(&mySpinlock);
	// portEXIT_CRITICAL(&mySpinlock);
	// When using this **in an interrupt handler,**use this instead:
	// portENTER_CRITICAL_ISR(&mySpinlock);
	// portEXIT_CRITICAL_ISR(&mySpinlock);
	// 
//	spinlock_initialize( &m_spinLock );

	return true;
}

void	AudioBuffer::GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount ) {

	// We know:
	//	• The time Tr of the current sample we're reading
	//	• The time Tw when the last sample was written to the buffer
	//	• The amount of samples between Tr and Tw
	// From this, we can deduce the sampling rate of this source, which is given by:
	//	Fs = N / (Tw - Tr)
	// 
	// Using the required sampling rate and amount of samples, we can compute Dt = _samplesCount / _samplingRate
	//	which is the amount of sampling time we need from the signal.
	// 
	// Finally, using Fs and Dt, we can easily compute how many samples we need to span:
	// 

//	float	bufferDeltaTime = (m_timeWrite - m_timeRead) / 1000000.0f;
	float	bufferDeltaTime = (max( m_timeRead+1, m_timeWrite ) - m_timeRead) / 1000000.0f;	// Always keep delta time positive, even if write time isn't increasing due to a loss of the transmitter feed


bufferDeltaTime = m_preloadDelay_Micros / 1000000.0f;	// Always for a perfect delay between read & write times


	float	bufferSamplesCount = m_sampleIndexWrite - m_sampleIndexRead;			// Integer part, should always be quite small (ideally, always equal to pre-load delay)
			bufferSamplesCount -= m_sampleIndexReadDecimal;							// Also account for decimal part from last read

	float	deltaTime = _samplesCount / _samplingRate;								// How much time are we asking for?
	float	sourceSamplesCount = bufferSamplesCount * deltaTime / bufferDeltaTime;	// How many source samples do we need to cover that much time?

// Here I tried to make read index simply reach write index - preload delay but write index keeps on growing at the same time, which accentuates wobbliness even worse!
float	preLoadDelaySamplesCount = GetSamplingRate() * (m_preloadDelay_Micros / 1000000.0f);
//Serial.printf( "%.1f + %.1f = %d | W %d - R %d = %f\n", m_bufferSize - preLoadDelaySamplesCount, preLoadDelaySamplesCount, m_bufferSize, U32(m_sampleIndexWrite), U32(m_sampleIndexRead), bufferSamplesCount );
//
////sourceSamplesCount = (m_sampleIndexWrite - preLoadDelaySamplesCount)	// <= Target position we want to reach after sampling
////					- (m_sampleIndexRead + m_sampleIndexReadDecimal);	// - current position
//sourceSamplesCount = bufferSamplesCount - preLoadDelaySamplesCount;		// Which is neatly simplified into this... <== SUPER WOBBLY!

// Interpolate between wobbly result and wobblier result... :/
sourceSamplesCount = 0.95f * sourceSamplesCount + (1.0f - 0.95f) * (bufferSamplesCount - preLoadDelaySamplesCount);

	float	sampleInc = sourceSamplesCount / _samplesCount;

	// Constrain read index to buffer size to save precision
	float	sampleIndexRead = m_sampleIndexRead % m_bufferSize;
			sampleIndexRead += m_sampleIndexReadDecimal;	// Add back decimal part that we were at last time

	// Perform sampling at required frequency
	if ( GetChannelsCount() == STEREO ) {
		for ( U32 sampleIndex=0; sampleIndex < _samplesCount; sampleIndex++, _samples++ ) {
			// Locate the 2 samples interval we must interpolate
			U32		intSampleIndexRead = U32( floor( sampleIndexRead ) );
			float	t = sampleIndexRead - intSampleIndexRead;	// In [0,1]

			U32		bufferSampleIndex = intSampleIndexRead % m_bufferSize;
			Sample*	sourceSample0 = m_buffer + bufferSampleIndex;
					bufferSampleIndex++;
			Sample*	sourceSample1 = bufferSampleIndex < m_bufferSize ? sourceSample0 + 1 : m_buffer;	// Wrap around if necessary

			// Interpolate values
			_samples->left = sourceSample0->left + t * (sourceSample1->left - sourceSample0->left);
			_samples->right = sourceSample0->right + t * (sourceSample1->right - sourceSample0->right);

			// Jump to next sample position
			sampleIndexRead += sampleInc;
		}
	} else {
		S16*	samples = (S16*) _samples;
		for ( U32 sampleIndex=0; sampleIndex < _samplesCount; sampleIndex++, samples++ ) {
			// Locate the 2 samples interval we must interpolate
			U32		intSampleIndexRead = U32( floor( sampleIndexRead ) );
			float	t = sampleIndexRead - intSampleIndexRead;	// In [0,1]

			U32		bufferSampleIndex = intSampleIndexRead % m_bufferSize;
			S16*	sourceSample0 = (S16*) m_buffer + bufferSampleIndex;
					bufferSampleIndex++;
			S16*	sourceSample1 = bufferSampleIndex < m_bufferSize ? sourceSample0 + 1 : (S16*) m_buffer;	// Wrap around if necessary

			// Interpolate values
			S16	value0 = *sourceSample0;
			S16	value1 = *sourceSample1;
			*samples = value0 + t * (value1 - value0);

			// Jump to next sample position
			sampleIndexRead += sampleInc;
		}
	}

	// Update new read index & time
	U64		intSourceSamplesCount = U64( floor( sourceSamplesCount ) );
	float	decimalSourceSamplesCount = sourceSamplesCount - intSourceSamplesCount;
	m_sampleIndexRead += intSourceSamplesCount;
	m_sampleIndexReadDecimal += decimalSourceSamplesCount;
	if ( m_sampleIndexReadDecimal >= 1.0f ) {
		m_sampleIndexReadDecimal -= 1.0f;
		m_sampleIndexRead++;
	}

//	m_timeRead = m_time->GetTimeMicros();
//	m_timeRead += U64( 1000000.0f * deltaTime );

	// Make sure the read index/time never goes beyond write index/time. When this happens, we just want to block the player.
	// This can happen if we lost the source and can't receive samples anymore, for example...
	if ( m_sampleIndexRead >= m_sampleIndexWrite ) {
		m_sampleIndexRead = m_sampleIndexWrite;
		m_sampleIndexReadDecimal = 0;
	}
//	m_timeRead = min( m_timeWrite-1, m_timeRead );

m_timeRead = m_time->GetTimeMicros();

//float	sourceSamplingRate = bufferSamplesCount / bufferDeltaTime;
//Serial.printf( "Read Index = %.3f / %d / %d - Read time = %.4f - Sampling Rate = %.3f - Samples Count = %.2f / %d (=%.4fs)\n", m_sampleIndexRead + m_sampleIndexReadDecimal, U32(m_sampleIndexWrite), m_bufferSize, m_timeRead / 1000000.0f, sourceSamplingRate, sourceSamplesCount, _samplesCount, deltaTime );
}

void	AudioBuffer::WriteSamples( const Sample* _samples, U32 _samplesCount ) {

//Serial.printf( "AudioBuffer::WriteSamples() => Time = %f + %f = %f\n", _time.GetTime(), m_preloadDelay, m_timeWrite );

	// Copy the new samples to the buffer
	U32	bufferSampleIndex = m_sampleIndexWrite % m_bufferSize;
	U32	samplesCountToEnd = m_bufferSize - bufferSampleIndex;

	if ( GetChannelsCount() == STEREO ) {
		if ( _samplesCount <= samplesCountToEnd ) {
			memcpy( m_buffer + bufferSampleIndex, _samples, _samplesCount * sizeof(Sample) );
		} else {	// Copy with wrap around
			memcpy( m_buffer + bufferSampleIndex, _samples, samplesCountToEnd * sizeof(Sample) );
			memcpy( m_buffer, _samples + samplesCountToEnd, (_samplesCount - samplesCountToEnd) * sizeof(Sample) );
		}
	} else {
		S16*	samples = (S16*) _samples;
		S16*	buffer = (S16*) m_buffer;
		if ( _samplesCount <= samplesCountToEnd ) {
			memcpy( buffer + bufferSampleIndex, samples, _samplesCount * sizeof(S16) );
		} else {	// Copy with wrap around
			memcpy( buffer + bufferSampleIndex, samples, samplesCountToEnd * sizeof(S16) );
			memcpy( buffer, samples + samplesCountToEnd, (_samplesCount - samplesCountToEnd) * sizeof(S16) );
		}
	}

	m_sampleIndexWrite += _samplesCount;

	// The time stamp must be written exactly at the end of the function
	m_timeWrite = m_time->GetTimeMicros() + m_preloadDelay_Micros;	// We always write ahead of replay time

/*
S16	minValue = 0, maxValue = 0;
for ( U32 i=0; i < _samplesCount; i++ ) {
	Sample&	sample = m_buffer[(bufferSampleIndex + m_bufferSize-i-1) % m_bufferSize];
	minValue = min( minValue, sample.left );
	maxValue = max( maxValue, sample.left );
}
Serial.printf( "Min = %d | Max = %d\n", minValue, maxValue );
//*/
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Default ITimeReference implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void	DefaultTime::Start() {
	// Write the time at which we start sampling
	#ifdef TIME_USE_XTAL
		m_XTalCountStart = U64( XTHAL_GET_CCOUNT() );
	#else
		m_timeStart = esp_timer_get_time();
	#endif
}

#ifdef TIME_USE_XTAL	// High-precision version using XTal but doesn't work right now => Keeps on going back to huge time values for some reason? Also a delay() between 2 successive calls to GetTime() isn't always properly counted
float	DefaultTime::GetTime() const {
	U64	XTalCountNow = XTHAL_GET_CCOUNT();
		XTalCountNow |= m_XTalCountLast & 0xFFFFFFFF00000000ULL;	// Keep last measurement's MSW
	if ( XTalCountNow - m_XTalCountLast < 0 ) {
		XTalCountNow += 0x100000000ULL;	// We looped!
	}
	m_XTalCountLast = XTalCountNow;

//	float	time = (XTalCountNow - m_XTalCountStart) / 240000000.0f;
//	float	time = float( XTalCountNow - m_XTalCountStart ) / F_CPU;	// Besoin de plus de précision ici !!!

	U64	deltaTime = XTalCountNow - m_XTalCountStart;
	U64	integerSeconds = deltaTime / U64( F_CPU );
	U32	decimalSeconds = U32( deltaTime - integerSeconds * U64( F_CPU ) );
	
	// F_CPU = 240,000,000 = 0xE4E1C00
	// Floats have only 24 bits of decimal precision so we can already shift by 4 bits to get more precision in our division
	float	time = float( decimalSeconds >> 4UL ) / (U32(F_CPU) >> 4UL)	// We're expecting a 2^(-23) seconds precision here
				 + integerSeconds;										// Which is about to get ruined by integer numbers leaving less room to encode decimals...

	return time;
}
#else
// Check https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/performance/speed.html for other possible precision measures
U64	DefaultTime::GetTimeMicros() const {
 	return esp_timer_get_time() - m_timeStart;
}
#endif
