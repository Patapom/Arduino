#include "../Global.h"
#include "WAVFileSampler.h"

WAVFileSampler::WAVFileSampler()
	: m_sampleIndex( 0 )
{
	memset( &m_sampleMin, 0, sizeof(Sample) );
	memset( &m_sampleMax, 0, sizeof(Sample) );
}
WAVFileSampler::~WAVFileSampler() {
	m_file.close();
}

bool    WAVFileSampler::Init( const char* _fileName ) {
Serial.println( "WAVFileSampler::Init()" );

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
    m_sampleIndex = 0;

    Serial.println( str( "WAV file is ready (%d bytes, current offset = %d)", header.fileSize+8, m_file.position() ) );
    Serial.println( str( "Data length = %d (%d samples at %.1f kHz - Duration %.3f s)", header.chunkSizeData, m_samplesCount, 0.001f * m_samplingRate, float(m_samplesCount) / m_samplingRate ) );

    return true;
}

// Fill the buffer with data from the file wrapping around if necessary
void    WAVFileSampler::GetSamples( Sample* _samples, U32 _samplesCount ) {
#if 1	// Read as many samples as we can in at most 2 read() calls
	U32		sampleSize = m_channelsCount * sizeof(S16);

	U8*		samplePtr = (U8*) _samples;
	U32		samplesCountToEOF = m_samplesCount - m_sampleIndex;
	U32		remainingSamplesCount = _samplesCount;
	while ( remainingSamplesCount > samplesCountToEOF ) {
		// Read to EOF and loop
		U32	readSize = samplesCountToEOF * sampleSize;
		m_file.read( samplePtr, readSize );
		m_file.seek( sizeof(WAVHeader) );

		samplePtr += readSize;
		remainingSamplesCount -= samplesCountToEOF;

Serial.println( str( "Loop! Offset %d / sampleIndex %d", m_file.position(), m_sampleIndex ) );

		// We now have the entire file ahead of us...
		m_sampleIndex = 0;
		samplesCountToEOF = m_samplesCount;
	}

	// Read the rest
	U32	readSize = remainingSamplesCount * sampleSize;
	m_file.read( samplePtr, readSize );
	samplePtr += readSize;
	m_sampleIndex += remainingSamplesCount;

	if ( m_channelsCount == CHANNELS::STEREO )
		return;	// We've already read stereo samples...

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

#else	// Read samples one by one...
	Sample*    sample = _samples;
	for ( int i=0; i < _samplesCount; i++, sample++ ) {
		// Read from file
		m_file.read( (U8*) &sample->left, sizeof(S16) );
		if ( m_channelsCount == 2 ) {
			m_file.read( (U8*) &sample->right, sizeof(S16) );
		} else {
			sample->right = sample->left;
		}
		m_sampleIndex++;

		// Keep min/max values
		if ( sample->left > m_sampleMax.left )   m_sampleMax.left = sample->left;
		if ( sample->right > m_sampleMax.right ) m_sampleMax.right = sample->right;
		if ( sample->left < m_sampleMin.left )   m_sampleMin.left = sample->left;
		if ( sample->right < m_sampleMin.right ) m_sampleMin.right = sample->right;

		// If we've reached the end of the file then seek back to the beginning (after the header)
		if ( m_sampleIndex == m_samplesCount ) {
//		if ( m_file.available() == 0 ) {
Serial.println( str( "Loop! Offset %d / sampleIndex %d (Min = 0x%04X - 0x%04x | Max = 0x%04X - 0x%04x)", m_file.position(), m_sampleIndex, m_sampleMin.left, m_sampleMin.right, m_sampleMax.left, m_sampleMax.right ) );
			m_file.seek( sizeof(WAVHeader) );
			m_sampleIndex = 0;
		}
	}
#endif
}
