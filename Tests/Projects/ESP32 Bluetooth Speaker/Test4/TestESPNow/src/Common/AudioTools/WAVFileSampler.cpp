#include "../Global.h"
#include "WAVFileSampler.h"

//#define SIMULATE_WAVE_FORM	1024	// Define this to substitute a perfect 1KHz waveform instead of the WAV content

WAVFileSampler::WAVFileSampler( const ITimeReference& _time ) : AudioBuffer( _time ) {
	memset( &m_sampleMin, 0, sizeof(Sample) );
	memset( &m_sampleMax, 0, sizeof(Sample) );
}
WAVFileSampler::~WAVFileSampler() {
	m_file.close();
}

bool    WAVFileSampler::Init( const char* _fileName, float _preLoadDelay ) {
//Serial.println( "WAVFileSampler::Init()" );

	m_file = SPIFFS.open( _fileName, "r", false );
	if ( !m_file ) {
		Serial.println( "Failed to open WAV file for reading!" );
		return false;
	}
	Serial.println( str( "WAV file \"%s\" opened!", _fileName ) );

	// Read header
	WAVHeader header;
	if ( m_file.read( (U8*) &header, sizeof(WAVHeader) ) != sizeof(WAVHeader) ) {
		Serial.println( "Failed to read WAV file header!" );
		return false;
	}

/*
Serial.println( "WAVHeader dump:" );
//    ESP_LOG_BUFFER_HEX( "POM", &header, sizeof(WAVHeader) );
//    esp_log_buffer_hexdump_internal( "POM", &header, sizeof(WAVHeader), ESP_LOG_WARN );
for ( U32 i=0; i < 6; i++ ) {
U8* p = ((U8*) &header)+ 8 * i;
Serial.println( str( "%02X%02X%02X%02X %02X%02X%02X%02X | %c%c%c%c%c%c%c%c", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7] ) );
}
//*/

// @TODO: Check if header chunk names are valid...

	if ( header.formatType != 1 ) {
		Serial.println( "WAV is not in PCM format! Can't read..." );
		return false;
	}
	if ( header.channelsCount > 2 ) {
		Serial.println( "WAV contains more than 2 channels! Can't read..." );
		return false;
	}
	if ( header.bitsPerMonoSample != 16 ) {
		Serial.println( "WAV is not using 16-bits samples..." );
		return false;
	}

	m_channelsCount = (CHANNELS) header.channelsCount;
	m_samplingRate = header.sampleRate;
	m_samplesCount = header.chunkSizeData >> 2;
	m_sampleSize = m_channelsCount * sizeof(S16);

	Serial.println( str( "WAV file is ready (%d bytes, current offset = %d)", header.fileSize+8, m_file.position() ) );
	Serial.println( str( "Data length = %d (%d samples at %.1f kHz - Duration %.3f s)", header.chunkSizeData, m_samplesCount, 0.001f * m_samplingRate, float(m_samplesCount) / m_samplingRate ) );

	// Allocate the audio buffer
	U32	preLoadSamplesCount = U32( ceil( _preLoadDelay * m_samplingRate ) );
	U32	bufferSize = preLoadSamplesCount + 8 * UPDATE_SAMPLES_COUNT;	// Give room for 8 updates before we loop back into "read territory"

 	return AudioBuffer::Init( bufferSize, preLoadSamplesCount, _preLoadDelay * 1000000.0f );
}

void	WAVUpdaterTask( void* _param ) {
	WAVFileSampler*	that = (WAVFileSampler*) _param;

	// Wait until time starts
	if ( !that->m_time->HasStarted() ) {
		vTaskDelay( 1 );
	}

	while ( true ) {
		that->Update();	// Update with some samples ASAP
//		if ( !digitalRead( PIN_BUTTON ) || that->Update() )	// Update with some samples ASAP
		vTaskDelay( 1 );
	}
}

void	WAVFileSampler::StartAutoUpdateTask( U8 _taskPriority ) {
	TaskHandle_t updateTaskHandle;
	xTaskCreate( WAVUpdaterTask, "WAVUpdaterTask", 2048, this, _taskPriority, &updateTaskHandle );
}

bool	WAVFileSampler::Update() {
	U64	now = m_time->GetTimeMicros();
	if ( now < m_timeNextUpdate )
		return false;	// Too soon!

	// Sadly, we have to use a temporary buffer
	// I could have rewritten the code to directly read from the file and into the buffer, but it involves handling 2 circular buffers (our buffer and the file) and I'm lazy... :/
	U32	sampleIndex = m_sampleIndexWrite % m_samplesCount;	// Constrain within the length of the file
	GetSamples( sampleIndex, m_temp, UPDATE_SAMPLES_COUNT );


#ifdef SIMULATE_WAVE_FORM	// Perfect sine wave when generated!
static U32	s_sampleIndex = 0;
if ( GetChannelsCount() == STEREO ) {
	for ( U32 i=0; i < UPDATE_SAMPLES_COUNT; i++ ) {
		S16	value = S16( SIMULATE_WAVE_FORM * sin( 2*PI * (1000.0 / m_samplingRate) * s_sampleIndex++ ) );
		m_temp[i].left = value;
		m_temp[i].right = value;
	}
} else {
	S16*	sample = (S16*) m_temp;
	for ( U32 i=0; i < UPDATE_SAMPLES_COUNT; i++ ) {
		*sample++ = S16( SIMULATE_WAVE_FORM * sin( 2*PI * (1000.0 / m_samplingRate) * s_sampleIndex++ ) );
	}
}
#endif


	WriteSamples( m_temp, UPDATE_SAMPLES_COUNT );

	// One more update!
	U64	updateDeltaTime = (1000000ULL * UPDATE_SAMPLES_COUNT) / U64( m_samplingRate );	// How much time represents that many samples given our sampling rate?
	m_timeNextUpdate += updateDeltaTime;
	m_updatesCount++;

	return true;
}

// Fill the buffer with data from the file, wrapping around if necessary
U32    WAVFileSampler::GetSamples( U32 _sampleIndex, Sample* _samples, U32 _samplesCount ) {
	// Read as many samples as we can in at most 2 read() calls
	U8*		samplePtr = (U8*) _samples;
	U32		samplesCountToEOF = m_samplesCount - _sampleIndex;
	U32		remainingSamplesCount = _samplesCount;
	while ( remainingSamplesCount > samplesCountToEOF ) {
		// Read to EOF and loop
		U32	readSize = samplesCountToEOF * m_sampleSize;
		m_file.read( samplePtr, readSize );
		m_file.seek( sizeof(WAVHeader) );

		samplePtr += readSize;
		remainingSamplesCount -= samplesCountToEOF;

//Serial.println( str( "Loop! Offset %d / sampleIndex %d", m_file.position(), _sampleIndex ) );

		// We now have the entire file ahead of us...
		_sampleIndex = 0;
		samplesCountToEOF = m_samplesCount;
	}

	// Read the rest
	U32	readSize = remainingSamplesCount * m_sampleSize;
	m_file.read( samplePtr, readSize );
	samplePtr += readSize;
	_sampleIndex += remainingSamplesCount;

	if ( m_channelsCount == CHANNELS::STEREO )
		return _samplesCount;	// We've already read stereo samples...

	if ( m_forceMono ) {
		// Convert stereo samples into mono
		Sample*	sampleSource = _samples;
		S16*	sampleTarget = (S16*) _samples;
		for ( U32 sampleIndex=0; sampleIndex < _samplesCount; sampleIndex++, sampleSource++ ) {
			*sampleTarget++ = sampleSource->left;
		}
		return _samplesCount;
	}

	// We need to double the mono samples to make them stereo...
	S16*	sourceSample = (S16*) samplePtr;
	Sample*	targetSample = _samples + _samplesCount;
	while ( targetSample > _samples ) {
		sourceSample--;
		targetSample--;
		targetSample->left = targetSample->right = *sourceSample;

		// Keep min/max values
		if ( targetSample->left > m_sampleMax.left )   m_sampleMax.left = targetSample->left;
		if ( targetSample->right > m_sampleMax.right ) m_sampleMax.right = targetSample->right;
		if ( targetSample->left < m_sampleMin.left )   m_sampleMin.left = targetSample->left;
		if ( targetSample->right < m_sampleMin.right ) m_sampleMin.right = targetSample->right;
	}
Serial.println( str( "Building %d stereo samples from mono (Min = 0x%04X - 0x%04x | Max = 0x%04X - 0x%04x)", _samplesCount, m_sampleMin.left, m_sampleMin.right, m_sampleMax.left, m_sampleMax.right ) );

	return _samplesCount;
}
