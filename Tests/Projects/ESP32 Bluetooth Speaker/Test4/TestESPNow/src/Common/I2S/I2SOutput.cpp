#include "../Global.h"
#include "I2SOutput.h"

#include "xtensa/core-macros.h"	// XTHAL_GET_CCOUNT()

// Here I tried various strategies:
//	• Calling WriteSamples() with the content of the packet every time we received a packet
//		=> Even simulating a perfect sine wave (i.e. no packet loss) induced some scratches
//	• Creating a task that listens to I2S events
//		=> This works perfectly when simulating a perfect sine wave
//		=> Still some scratches when attempting read perfect sine wave from buffer of received packets!
//		=> But no more scratches if we packets containing a perfect sine wave generated on the transmitter side!
//		=> All in all, the main issue seems to be with a buffer overrun issue by the player catching up to the receiver buffer...
//
void I2SWriterTask( void* _param ) {
	I2SOutput*	that = (I2SOutput*) _param;

	Sample*		samples = (Sample*) malloc( I2SOutput::DMA_SIZE * sizeof(Sample) );
	memset( samples, 0, I2SOutput::DMA_SIZE * sizeof(Sample) );

	// Wait until the reference time starts
	while ( !that->m_time->HasStarted() ) {
		delay( 1 );
	}

	while ( true ) {
		// wait for some data to be requested
		i2s_event_t evt;
		if ( xQueueReceive( that->m_I2SQueue, &evt, portMAX_DELAY ) != pdPASS )
			continue;
		if ( evt.type != I2S_EVENT_TX_DONE )
			continue;

		that->GetSamplesFromSource( samples, I2SOutput::DMA_SIZE );
		that->WriteSamples( samples, I2SOutput::DMA_SIZE );
	}
}

void I2SOutput::Init( i2s_port_t _I2SPort, const i2s_pin_config_t& _pins, ISampleSource& _sampleSource, U32 _samplingRate ) {
	m_sampleSource = &_sampleSource;
	m_samplingRate = _samplingRate;

	// i2s config for writing both channels of I2S
	i2s_config_t config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
		.sample_rate = m_samplingRate,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
//  	.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S), Obsolete
		.communication_format = i2s_comm_format_t( I2S_COMM_FORMAT_STAND_I2S ),
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
		.dma_buf_count = 4,
		.dma_buf_len = DMA_SIZE,	//1024
	};

	m_I2SPort = _I2SPort;

	// Install and start I2S driver
	ERROR( ESP_OK != i2s_driver_install( m_I2SPort, &config, 4, &m_I2SQueue ), "Failed to install I2S driver!" );
	ERROR( ESP_OK != i2s_set_pin( m_I2SPort, &_pins ), "Failed to setup the I2S external pins!" );
	ERROR( ESP_OK != i2s_zero_dma_buffer( m_I2SPort ), "Failed to clear the DMA buffers!" );   // Clear the DMA buffers

	// Start a task to write samples to the I2S peripheral
	xTaskCreate( I2SWriterTask, "I2S Writer Task", 4096, this, 1, &m_I2STaskHandle );	// 1KB & 2KB are dodgy (exception on launch *sometimes*), 4KB works...
}

U32	I2SOutput::WriteSamples( Sample* _samples, U32 _samplesCount ) {
	size_t	bytesWritten;
	i2s_write( m_I2SPort, _samples, _samplesCount * sizeof(Sample), &bytesWritten, portMAX_DELAY );
	if ( (bytesWritten % sizeof(Sample)) != 0 ) throw "Unexpected amount of bytes written! Should be integer samples...";
	return U32( bytesWritten / sizeof(Sample) );
}

void	I2SOutput::GetSamplesFromSource( Sample* _samples, U32 _requestedSamplesCount ) {


#if 0	// Perfect sine wave when requested from I2S task!
static U32	s_sampleIndex = 0;
for ( U32 i=0; i < _requestedSamplesCount; i++ ) {
//	S16	value = S16( 64 * sin( 2*PI * (1000.0 / m_samplingRate) * s_sampleIndex++ ) );
	S16	value = FastSine( s_sampleIndex++ * (16384 * 1000 / m_samplingRate) ) / 64;
	_samples[i].left = value;
	_samples[i].right = value;
}
return;
#endif


	// Get some samples from the audio buffer
	m_sampleSource->GetSamples( m_samplingRate, _samples, _requestedSamplesCount );

	// Should we convert mono source to stereo? (only useful when feeding back microphone into speakers really...)
	if ( m_sampleSource->GetChannelsCount() == ISampleSource::MONO ) {
		S16*	monoSample = (S16*) _samples + _requestedSamplesCount;
		Sample*	stereoSample = _samples + _requestedSamplesCount;
		for ( U32 sampleIndex=0; sampleIndex < _requestedSamplesCount; sampleIndex++ ) {
			monoSample--;
			stereoSample--;
			stereoSample->left = stereoSample->right = *monoSample;
		}
	}

	if ( m_volumeInt != 0xFFFF ) {
		// Apply volume
		Sample*	sample = _samples;
		for ( U32 sampleIndex=0; sampleIndex < _requestedSamplesCount; sampleIndex++, sample++ ) {
			sample->left = S16( (S32(sample->left) * m_volumeInt) >> 16 );
			sample->right = S16( (S32(sample->right) * m_volumeInt) >> 16 );
		}
	}
}

