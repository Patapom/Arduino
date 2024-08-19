#include "Global.h"
#include "AudioBuffer.h"
#include <freertos/FreeRTOS.h>

AudioBuffer::AudioBuffer()
    : m_sampleSource( NULL )
	, m_sampleIndex( 0 )
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

// Use this for a replay of the WAV file using I2S, it's okay to use _autoPreLoad and the SPIFFS inside the callback
/*bool	AudioBuffer::InitForI2SReplay( const char* _fileName ) {
	return Init( _fileName, I2SOutput::NUM_FRAMES_TO_SEND, true );
}

// When dealing with transferring samples to ESP-Now, we can't use SPIFFS but we can use the main thread to preload large amounts of samples
bool	AudioBuffer::InitForESPNow( const char* _fileName ) {
	return Init( _fileName, 4 * AudioBufferESPNow::MAX_ESP_NOW_PACKET_SIZE, false );
}
*/

bool	AudioBuffer::Init( ISampleSource& _sampleSource, U32 _bufferSamplesCount, bool _autoPreLoad ) {
//Serial.println( "AudioBuffer::Init()" );

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

	// Fill the buffer with pre-loaded samples
	m_sampleIndex = 0;
	m_preloadedSamplesCount = 0;
	if ( !UpdateBuffer( m_bufferSize ) )
		return false;

	Serial.println( str( "AudioBuffer::Init() => Successfully updated buffer with %d samples!", m_bufferSize ) );
	
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

bool	AudioBuffer::UpdateBuffer( U32 _requestedSamplesCount ) {
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

	// So the idea here is to pre-load some data in advance of the current sample "cursor" (i.e. m_sampleIndex)
	// The amount of samples that have currently been preloaded is stored in m_preloadedSamplesCount
	//
	// As soon as _requestedSamplesCount samples have been played and have become useless, they should get replaced by new samples...
	//
	U32	loadSampleIndex = m_preloadedSamplesCount	// Where we are now (after the last sample we loaded)
						- m_bufferSize;				// And we want to load at the beginning of the buffer, as far as we can behind us
	if ( m_sampleIndex - loadSampleIndex < _requestedSamplesCount ) {

#ifdef DEBUG_LOG
m_log[U8( m_logIndex + 0xFF)] |= 0x0F000000UL;	// Indicate that we tried to preload but got refused...
#endif

		return false;	// We're still busy using samples within that segment...
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
	U32	samplesCount = min( m_bufferSize - loadSampleIndex, _requestedSamplesCount );	// Either the end of the buffer comes first, or the end of the requested samples...
	m_sampleSource->GetSamples( m_buffer + loadSampleIndex, samplesCount );
	m_preloadedSamplesCount += samplesCount;
	_requestedSamplesCount -= samplesCount;

	if ( _requestedSamplesCount > 0 ) {
		// Wrap around the buffer and read the rest...
		m_sampleSource->GetSamples( m_buffer, _requestedSamplesCount );
		m_preloadedSamplesCount += _requestedSamplesCount;
	}

#ifdef DEBUG_LOG
m_log[m_logIndex++] = 0xF0000000UL | (m_preloadedSamplesCount - m_sampleIndex);
#endif

	return true;
}

// Fill the caller with data from the buffer
void    AudioBuffer::GetSamples( Sample* _samples, U32 _requestedSamplesCount ) {
	// When we manually update the buffer on the main thread, we must be able to provide the samples all at once.
	// In order to do so, the pre-load buffer must be at least twice as long as the requested amount of samples...
//	ERROR( !m_autoPreLoad && (2*_requestedSamplesCount) > m_bufferSize, "The pre-load buffer is not large enough to provide the requested amount of samples!" );

	while ( _requestedSamplesCount > 0 ) {
		// Feed as many samples as are available in the buffer
		U32	availableSamplesCount = m_preloadedSamplesCount - m_sampleIndex;

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
			return;
		}

		U32	samplesCount = min( availableSamplesCount, _requestedSamplesCount );
		U32	loadedSamplesCount = 0;
		U32	bufferSampleIndex = m_sampleIndex % m_bufferSize;		// Constrain within the buffer
		U32	samplesCountToEnd = m_bufferSize - bufferSampleIndex;	// How many can we load until we reach the end of the buffer?
		if ( samplesCountToEnd < samplesCount ) {
			// Copy to the end of the buffer then loop around
			memcpy( _samples, m_buffer + bufferSampleIndex, samplesCountToEnd * sizeof(Sample) );
			_samples += samplesCountToEnd;
			loadedSamplesCount += samplesCountToEnd;
			samplesCount -= samplesCountToEnd;

			bufferSampleIndex = 0;
		}

		memcpy( _samples, m_buffer + bufferSampleIndex, samplesCount * sizeof(Sample) );
		_samples += samplesCount;
		loadedSamplesCount += samplesCount;

		// Advance the sample cursor and decrease the amount of requested samples by the amount we just loaded
		m_sampleIndex += loadedSamplesCount;
		_requestedSamplesCount -= loadedSamplesCount;

#ifdef DEBUG_LOG
m_log[m_logIndex++] = m_preloadedSamplesCount - m_sampleIndex;
#endif

		if ( m_autoPreLoad ) {
			UpdateBuffer( loadedSamplesCount );	// Pre-load next samples to replace the one we just used...
		}
	}
}
