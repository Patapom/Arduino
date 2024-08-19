#include "../Global.h"
#include "I2SOutput.h"

void I2SWriterTask( void* _param ) {
	I2SOutput*  output = (I2SOutput*) _param;
	Sample*		samples = (Sample*) malloc( I2SOutput::NUM_SAMPLES_TO_SEND * sizeof(Sample) ); // 2 KB
	int         availableBytes = 0;
	uint8_t*    buffer = NULL;

	while ( true ) {
		// wait for some data to be requested
		i2s_event_t evt;
		if ( xQueueReceive( output->m_I2SQueue, &evt, portMAX_DELAY) != pdPASS )
			continue;
		if ( evt.type != I2S_EVENT_TX_DONE )
			continue;

		size_t	bytesWritten = 0;
		U32		totalBytesWritten = 0;
		do {
			if ( availableBytes == 0 ) {
				U32	availableSamples = output->GetSamples( samples );
				availableBytes = availableSamples * sizeof(Sample);	// How many bytes do we now have to send
				buffer = (uint8_t*) samples;
			}

			// Write data to the I2S peripheral
			i2s_write( output->m_I2SPort, buffer, availableBytes, &bytesWritten, portMAX_DELAY );
			availableBytes -= bytesWritten;
			buffer += bytesWritten;
			totalBytesWritten += bytesWritten;

//if ( totalBytesWritten > 1000 ) {
//Serial.println( str( "Wrote %d bytes to I2S", totalBytesWritten ) );
//totalBytesWritten = 0;
//}

		} while ( bytesWritten > 0 );
// Turns out it never exits the loop!

//Serial.println( str( "Wrote %d bytes to I2S", totalBytesWritten ) );
	}
}

void I2SOutput::Start( i2s_port_t _I2SPort, const i2s_pin_config_t& _pins, ISampleSource& _sampleSource ) {
	m_sampleSource = &_sampleSource;

	// i2s config for writing both channels of I2S
	i2s_config_t config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
		.sample_rate = m_sampleSource->GetSamplingRate(),
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
//  	.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S), Obsolete
		.communication_format = i2s_comm_format_t( I2S_COMM_FORMAT_STAND_I2S ),
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
		.dma_buf_count = 4,
		.dma_buf_len = 1024
	};

	m_I2SPort = _I2SPort;

	i2s_driver_install( m_I2SPort, &config, 4, &m_I2SQueue );    //install and start i2s driver
	i2s_set_pin( m_I2SPort, &_pins ); // set up the i2s pins
	i2s_zero_dma_buffer( m_I2SPort );   // clear the DMA buffers

	// start a task to write samples to the I2S peripheral
	TaskHandle_t writerTaskHandle;
	xTaskCreate( I2SWriterTask, "I2S Writer Task", 4096, this, 1, &writerTaskHandle );
}

U32	I2SOutput::GetSamples( Sample* _samples ) {
	m_sampleSource->GetSamples( _samples, NUM_SAMPLES_TO_SEND );	// Get some samples from the audio buffer

	if ( m_volumeInt != 0xFFFF ) {
		// Apply volume
		Sample*	sample = _samples;
		for ( U32 sampleIndex=0; sampleIndex < NUM_SAMPLES_TO_SEND; sampleIndex++, sample++ ) {
			sample->left = S16( (S32(sample->left) * m_volumeInt) >> 16 );
			sample->right = S16( (S32(sample->right) * m_volumeInt) >> 16 );
		}
	}

	return NUM_SAMPLES_TO_SEND;
}