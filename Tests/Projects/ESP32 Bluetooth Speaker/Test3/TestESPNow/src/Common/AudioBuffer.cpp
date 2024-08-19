#include "Global.h"
#include "AudioBuffer.h"
#include <freertos/FreeRTOS.h>

AudioBuffer::AudioBuffer()
    : m_sampleSource( NULL )
	, m_sampleIndexRead( 0 )
	, m_bufferSize( 0 )
    , m_buffer( NULL )
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

bool	AudioBuffer::Init( ISampleSource& _sampleSource, U32 _bufferSamplesCount, bool _autoPreLoad ) {
	m_sampleSource = &_sampleSource;

	// Allocate pre-load buffer
	m_autoPreLoad = _autoPreLoad;
	m_bufferSize = _bufferSamplesCount;

	if ( m_buffer != NULL ) {
		free( m_buffer );	// Clear any existing buffer...
	}

//	m_buffer = new Sample[m_bufferSize];
	m_buffer = (Sample*) malloc( m_bufferSize * sizeof(Sample) );
	ERROR( m_buffer == NULL, "Failed to allocate pre-load buffer! (out of memory)" );

	m_sampleIndexRead = 0;
	m_sampleIndexWrite = 0;
	if ( !m_autoPreLoad ) {
		Serial.println( "AudioBuffer::Init() => Buffer is ready with 0 samples!" );
		return true;
	}

	// Fill the buffer with pre-loaded samples
	U32	initialSamplesCount = UpdateBuffer( m_bufferSize );

	Serial.println( str( "AudioBuffer::Init() => Successfully updated buffer with %d samples!", initialSamplesCount ) );

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

U32	AudioBuffer::UpdateBuffer( U32 _requestedSamplesCount ) {
	ERROR( _requestedSamplesCount > m_bufferSize, str( "Buffer is not large enough to provide the requested %d samples! Try and allocate a buffer at least twice as big as the maximum amount of requested samples...", _requestedSamplesCount ) );

#ifdef DEBUG_LOG
/*
	// Display log of sample cursor gaps...
	static U8	previousLogIndex = 0x00;
	U32	logIndex = m_logIndex;
	if ( m_logIndex < previousLogIndex ) {
		U32	temp[256];
		memcpy( temp, m_log, 256 * sizeof(U32) );

		U32*	p = temp;
		for ( U32 i=0; i < 32; i++ ) {
			Serial.println( str( "%08X %08X %08X %08X %08X %08X %08X %08X", *p++, *p++, *p++, *p++, *p++, *p++, *p++, *p++ ) );
		}
		Serial.println();
	}
	previousLogIndex = logIndex;
//*/

	// Display the amount of starvation events that occurred in a second
	static U32	previousStartvationEventsCount = 0;
	static U32	previousMillis = millis();
	U32	now = millis();
	if ( now - previousMillis >= 1000 ) {
		Serial.println( str( "Starvation events %d (%f seconds)", m_starvationEventsCount, (1000.0f * (m_starvationEventsCount - previousStartvationEventsCount)) / (now - previousMillis) ) );
		previousMillis = now;
		previousStartvationEventsCount = m_starvationEventsCount;
		m_starvationEventsCount = 0;
	}
#endif

	// The idea here is to pre-load some data in advance of the current sample "cursor" (i.e. m_sampleIndex)
	// The amount of samples that have currently been preloaded is stored in m_preloadedSamplesCount
	//
	// As soon as _requestedSamplesCount samples have been played and have become useless, they should get replaced by new samples...
	//
	U32	loadSampleIndex = m_sampleIndexWrite	// Where we are now (after the last sample we loaded)
						- m_bufferSize;			// And we want to load at the beginning of the buffer, as far as we can behind us
	if ( m_sampleIndexRead - loadSampleIndex < _requestedSamplesCount ) {

#ifdef DEBUG_LOG
m_log[U8( m_logIndex + 0xFF)] |= 0x0F000000UL;	// Indicate that we tried to preload but got refused...
#endif

		return 0;	// We're still busy using samples within that segment...
	}

// Indicate a buffer update event
//if ( !xSemaphoreTake( m_semaphore, portMAX_DELAY ) )
//	return false;
//portENTER_CRITICAL( &m_spinLock );
//m_log[m_logIndex-1] |= 0xF0000000UL;
//m_log[m_logIndex-1] |= _requestedSamplesCount << 16;
//m_log[U8( m_logIndex + 0xFF)] |= 0xF0000000UL;
//m_log[U8( m_logIndex + 0xFF)] |= _requestedSamplesCount << 16;
//portEXIT_CRITICAL( &m_spinLock );
//xSemaphoreGive( m_semaphore );

//Serial.println( str( "Pre-loaded buffer with %d samples!", _requestedSamplesCount ) );

	// Copy as much as we can until either the requested amount of samples, or the end of the preload buffer
	loadSampleIndex %= m_bufferSize;	// Constrain index to stay within the buffer

	U32	readSamplesCount = 0;
	U32	samplesCountToEnd = m_bufferSize - loadSampleIndex;
	if ( _requestedSamplesCount <= samplesCountToEnd ) {	// Read in a single shot...
		readSamplesCount = m_sampleSource->GetSamples( m_buffer + loadSampleIndex, _requestedSamplesCount );
	} else {	// Read in 2 shots after looping through the buffer
		readSamplesCount = m_sampleSource->GetSamples( m_buffer + loadSampleIndex, samplesCountToEnd );
		readSamplesCount += m_sampleSource->GetSamples( m_buffer, _requestedSamplesCount - samplesCountToEnd );
	}
	m_sampleIndexWrite += readSamplesCount;

#ifdef DEBUG_LOG
m_log[m_logIndex++] = 0xF0000000UL | (m_sampleIndexWrite - m_sampleIndexRead);
#endif

	return readSamplesCount;
}

// Fill the caller with data from the buffer
U32    AudioBuffer::GetSamples( Sample* _samples, U32 _requestedSamplesCount ) {
	// When we manually update the buffer on the main thread, we must be able to provide the samples all at once.
	// In order to do so, the pre-load buffer must be at least twice as long as the requested amount of samples...
//	ERROR( !m_autoPreLoad && (2*_requestedSamplesCount) > m_bufferSize, "The pre-load buffer is not large enough to provide the requested amount of samples!" );

	U32	loadedSamplesCount = 0;
	while ( _requestedSamplesCount > 0 ) {
		// Feed as many samples as are available in the buffer
		U32	availableSamplesCount = m_sampleIndexWrite - m_sampleIndexRead;

//if ( !xSemaphoreTake( m_semaphore, portMAX_DELAY ) )
//	return;
//portENTER_CRITICAL( &m_spinLock );
//m_log[m_logIndex++] = availableSamplesCount;
//portEXIT_CRITICAL( &m_spinLock );
//xSemaphoreGive( m_semaphore );

//		ERROR( availableSamplesCount == 0, "Buffer starved of samples!" );
		if ( availableSamplesCount == 0 ) {
//Serial.println( "Buffer starved of samples!" );
#ifdef DEBUG_LOG
m_log[m_logIndex++] = 0;
m_starvationEventsCount++;
#else
		// If this happens, you can enable the DEBUG_LOG to see how many starvation events occur and try increasing the size of the buffer
		// You can also enable verbose log debugging to watch how closely the feeder/consumer are close together but know that simply
		//	Serial.printing the log takes enough time to increase the starvation issue!
//		ERROR( true, "Buffer starved of samples!" );
#endif
			return loadedSamplesCount;
		}

		U32	samplesCount = min( availableSamplesCount, _requestedSamplesCount );
		U32	bufferSampleIndex = m_sampleIndexRead % m_bufferSize;		// Constrain within the buffer
		U32	samplesCountToEnd = m_bufferSize - bufferSampleIndex;	// How many can we load until we reach the end of the buffer?
		if ( samplesCount < samplesCountToEnd  ) {
			// Copy in a single shot
			memcpy( _samples, m_buffer + bufferSampleIndex, samplesCount * sizeof(Sample) );
		} else {
			// Copy to the end of the buffer then loop around
			memcpy( _samples, m_buffer + bufferSampleIndex, samplesCountToEnd * sizeof(Sample) );
			memcpy( _samples + samplesCountToEnd, m_buffer, (samplesCount - samplesCountToEnd) * sizeof(Sample) );
		}

		// Advance the sample cursor and decrease the amount of requested samples by the amount we just loaded
		_samples += samplesCount;
		m_sampleIndexRead += samplesCount;
		_requestedSamplesCount -= samplesCount;
		loadedSamplesCount += samplesCount;

#ifdef DEBUG_LOG
m_log[m_logIndex++] = m_preloadedSamplesCount - m_sampleIndex;
#endif

		if ( m_autoPreLoad ) {
			UpdateBuffer( samplesCount );	// Pre-load next samples to replace the one we just used...
		}
	}

	return loadedSamplesCount;
}

// Copy samples from a source circular buffer to a target linear buffer
void	AudioBuffer::CopyFromCircularBuffer( const Sample* _sourceCircularBuffer, U32 _circularBufferSize, U32& _sourceIndex, Sample* _targetBuffer, U32 _samplesCount ) {
	ERROR( _samplesCount > _circularBufferSize, "Circular buffer is too small to provide the required amount of samples!" );
throw "TODO!";
}

// Copy samples from a source linear buffer to a target circular buffer
void	AudioBuffer::CopyToCircularBuffer( const Sample* _sourceBuffer, Sample* _targetCircularBuffer, U32 _circularBufferSize, U32& _targetIndex, U32 _samplesCount ) {
	ERROR( _samplesCount > _circularBufferSize, "Circular buffer is too small to receive the required amount of samples!" );
throw "TODO!";
}
