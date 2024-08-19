#include "../Global.h"
#include "WAVFileSampler.h"

WAVFileSampler::WAVFileSampler()
    : m_sampleIndex( 0 )
    , m_fileContent( NULL )
{
    memset( &m_frameMin, 0, sizeof(Frame_t) );
    memset( &m_frameMax, 0, sizeof(Frame_t) );
    SetVolume( 1 );
}
WAVFileSampler::~WAVFileSampler() {
    if ( m_fileContent != NULL ) {
        free( m_fileContent );
    } else {
        m_file.close();
    }
}

bool    WAVFileSampler::Init( const char* _fileName, bool _preLoadFile ) {
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

    m_channelsCount = header.channelsCount;
    m_samplingRate = header.sampleRate;
    m_samplesCount = header.chunkSizeData >> 2;
    m_sampleIndex = 0;

    Serial.println( str( "WAV file is ready (%d bytes, current offset = %d)", header.fileSize+8, m_file.position() ) );
    Serial.println( str( "Data length = %d (%d samples at %.1f kHz - Duration %.3f s)", header.chunkSizeData, m_samplesCount, 0.001f * m_samplingRate, float(m_samplesCount) / m_samplingRate ) );

    // Pre-load the file's content? (faster access)
    if ( _preLoadFile ) {

//m_samplesCount = 25000;
//header.chunkSizeData = m_samplesCount * 4;

        m_fileContent = (U8*) malloc( header.chunkSizeData );
        if ( m_fileContent != NULL ) {
            m_file.read( m_fileContent, header.chunkSizeData );
            m_file.close();
            Serial.println( str( "File content pre-loaded! (Address = %08X)", U32(m_fileContent) ) );
        } else {
            Serial.println( str( "FAILED to  pre-load file content! Size %08X is too large...", header.chunkSizeData ) );
        }
    }

    return true;
}

// fill the buffer with data from the file wrapping around if necessary
void    WAVFileSampler::getFrames( Frame_t* _frames, int _framesCount ) {
/*    if ( m_fileContent != NULL ) {
        U32 sampleSize = m_channelsCount*sizeof(S16);

        // Check if we're looping?
        if ( m_sampleIndex + _framesCount > m_samplesCount ) {
            U32 countToEnd = m_samplesCount - m_sampleIndex;
            if ( countToEnd > 0 ) {
                memcpy( _frames, m_fileContent + m_sampleIndex * sampleSize, countToEnd * sampleSize );
            }

            // Restart...
            m_sampleIndex = 0;
            _framesCount -= countToEnd;
        }

        if ( _framesCount > 0 ) {
            memcpy( _frames, m_fileContent + m_sampleIndex * sampleSize, _framesCount * sampleSize );
        }

        return;
    }
*/
    Frame_t*    frame = _frames;
    for ( int i=0; i < _framesCount; i++, frame++ ) {
        // Read in the next sample to the left & right channels
        S16 left = 0, right = 0;
        if ( m_fileContent != NULL ) {
            // Copy from memory
            U32 sampleSize = m_channelsCount*sizeof(S16);
            memcpy( frame, m_fileContent + m_sampleIndex * sampleSize, sampleSize );
        } else {
            // Read from file
            m_file.read( (U8*) &left, sizeof(S16) );
            if ( m_channelsCount == 2 ) {
                m_file.read( (U8*) &right, sizeof(S16) );
            } else {
                right = left;
            }
        }
        m_sampleIndex++;

        // Apply volume
        frame->left = U16( (S32(left) * m_volumeInt) >> 16 );
        frame->right = U16( (S32(right) * m_volumeInt) >> 16 );

        // Keep min/max values
        if ( frame->left > m_frameMax.left )   m_frameMax.left = frame->left;
        if ( frame->right > m_frameMax.right ) m_frameMax.right = frame->right;
        if ( frame->left < m_frameMin.left )   m_frameMin.left = frame->left;
        if ( frame->right < m_frameMin.right ) m_frameMin.right = frame->right;

        // If we've reached the end of the file then seek back to the beginning (after the header)
        if ( m_sampleIndex == m_samplesCount ) {
//        if ( m_file.available() == 0 ) {
Serial.println( str( "Loop! Offset %d / sampleIndex %d (Min = 0x%04X - 0x%04x | Max = 0x%04X - 0x%04x)", m_file.position(), m_sampleIndex, m_frameMin.left, m_frameMin.right, m_frameMax.left, m_frameMax.right ) );
            m_file.seek( sizeof(WAVHeader) );
            m_sampleIndex = 0;
        }
    }
}
