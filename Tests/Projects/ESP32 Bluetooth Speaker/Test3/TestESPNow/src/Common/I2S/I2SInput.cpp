#include "../Global.h"
#include "I2SInput.h"

// I2S DAC sampling reads 12-bit values that need to be r
void I2SReaderTask_DAC( void* _param ) {
	I2SInput*	that = (I2SInput*) _param;
	while ( true ) {
		// Wait for some data to arrive on the queue
		i2s_event_t evt;
		if ( xQueueReceive( that->m_I2SQueue, &evt, portMAX_DELAY) != pdPASS )
			continue;
		if ( evt.type != I2S_EVENT_RX_DONE )
			continue;

		while ( true ) {
			// Try and fill up our audio buffer
			U32		sampleIndexWrite = that->m_sampleIndexWrite % that->m_bufferSize;
			U32		samplesCount = that->m_bufferSize - sampleIndexWrite;

			size_t	bytesRead = 0;
			i2s_read( that->m_I2SPort, that->m_buffer + sampleIndexWrite, samplesCount * sizeof(U16), &bytesRead, 10 / portTICK_PERIOD_MS );
			if ( bytesRead == 0 )
				break;

			// Expand 12-bits unsigned mono samples into 16-bits signed mono samples
			S16*	sample = that->m_buffer + sampleIndexWrite;
			U32		readSamplesCount = bytesRead >> 1;
			for ( U32 i=0; i < readSamplesCount; i++ ) {
				S16	value = clamp( *sample, S16(0), S16(4095) );	// In [0,4095]
					value = value - 2048;							// Now in [-2048,2047]
					value *= 16;									// Now in [-32768,32752]
				*sample++ = value;
that->m_sampleMin = min( that->m_sampleMin, value );
that->m_sampleMax = max( that->m_sampleMax, value );
			}
			
			that->m_sampleIndexWrite += readSamplesCount;
		}
	}
}

// I2S MEMS like the INMP411 send 24-bits signe samples inside a 32-bits word
void I2SReaderTask_I2S( void* _param ) {
	I2SInput*	that = (I2SInput*) _param;
	S32			tempBuffer[256];
	while ( true ) {
		// Wait for some data to arrive on the queue
		i2s_event_t evt;
		if ( xQueueReceive( that->m_I2SQueue, &evt, portMAX_DELAY) != pdPASS )
			continue;
		if ( evt.type != I2S_EVENT_RX_DONE )
			continue;

		while ( true ) {
			// Try and fill up our audio buffer
			U32		sampleIndexWrite = that->m_sampleIndexWrite % that->m_bufferSize;
			U32		samplesCount = that->m_bufferSize - sampleIndexWrite;
					samplesCount = min( 256UL, samplesCount );	// We only have 256 entries max in our temp buffer!

			size_t	bytesRead = 0;
			i2s_read( that->m_I2SPort, tempBuffer, samplesCount * sizeof(U32), &bytesRead, 10 / portTICK_PERIOD_MS );
			if ( bytesRead == 0 )
				break;
			if ( bytesRead & 3 )
				throw "Didn't read an integral number of samples!!";

			U32	shift = 16 - that->m_gainLog2;

			// Compress 24-bits signed mono samples into 16-bits signed mono samples
			S32*	sampleSource = tempBuffer;	// 24-bit signed values [-8388608, 8388607] in the top 3 MSB
			S16*	sampleTarget = that->m_buffer + sampleIndexWrite;
			U32		readSamplesCount = bytesRead >> 2;
			for ( U32 i=0; i < readSamplesCount; i++ ) {
//Serial.printf( "%08X ", *sampleSource );
//Serial.printf( "%04X ", ((S16*) sampleSource)[1] );
//Serial.printf( "%6d ", *sampleSource );

//				*sampleTarget++ = ((S16*) sampleSource)[1];		// In [-32768,32767]

				*sampleTarget++ = S16( *sampleSource >> shift );	// In [-32768,32767]
//				*sampleTarget++ = S16( *sampleSource++ / 65536 );	// In [-32768,32767]
//				*sampleTarget++ = *((S16*) &((U8*) sampleSource)[1]);	// In [-32768,32767]
				sampleSource++;

//				U8*	temp = (U8*) sampleSource;
//				*sampleTarget++ = S16( (temp[2] << 8) | temp[1] );
//				sampleSource++;

that->m_sampleMin = min( that->m_sampleMin, sampleTarget[-1] );
that->m_sampleMax = max( that->m_sampleMax, sampleTarget[-1] );
			}

			that->m_sampleIndexWrite += readSamplesCount;
		}
	}
}

I2SInput::I2SInput() {
}
I2SInput::~I2SInput() {
	delete[] m_buffer;
}

void I2SInput::Start( i2s_port_t _I2SPort, U32 _samplingRate, U32 _bufferSize, bool _ADCMode ) {
	m_samplingRate = _samplingRate;

	m_bufferSize = _bufferSize;
	m_buffer = new S16[m_bufferSize];
	memset( m_buffer, 0, m_bufferSize*sizeof(S16) );

	i2s_config_t i2s_config = {
		.mode = (i2s_mode_t) (_ADCMode ? I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN
									   : I2S_MODE_MASTER | I2S_MODE_RX),

//		.sample_rate = m_samplingRate,


// When testing feedback loop and mono mic is directly used as source of stereo speakers...
.sample_rate = 2*m_samplingRate,


		.bits_per_sample = _ADCMode ? I2S_BITS_PER_SAMPLE_16BIT
									: I2S_BITS_PER_SAMPLE_32BIT,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
//		.communication_format = I2S_COMM_FORMAT_I2S_LSB,	// Obsolete
		.communication_format = i2s_comm_format_t( _ADCMode ? I2S_COMM_FORMAT_STAND_MSB
															: I2S_COMM_FORMAT_STAND_I2S ),
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
//		.dma_buf_count = 2,
//		.dma_buf_len = 1024,
		.dma_buf_count = 4,
		.dma_buf_len = 64,
		.use_apll = false,
		.tx_desc_auto_clear = false,
		.fixed_mclk = 0
	};

	m_I2SPort = _I2SPort;

	// Install and start I2S driver
	ERROR( ESP_OK != i2s_driver_install( m_I2SPort, &i2s_config, 4, &m_I2SQueue ), "Failed to install I2S driver!" );
}

void I2SInput::StartADC( i2s_port_t _I2SPort, U32 _samplingRate, U32 _bufferSize, adc1_channel_t _ADCChannel ) {
	ERROR( m_I2SPort != I2S_NUM_0, "I2S built-in ADC/DAC only supported on I2S0!" );
	Start( _I2SPort, _samplingRate, _bufferSize, true );

	// Init ADC pad
	ERROR( ESP_OK != i2s_set_adc_mode( ADC_UNIT_1, _ADCChannel ), "Failed setting ADC mode!" );
	ERROR( ESP_OK != i2s_zero_dma_buffer( m_I2SPort ), "Failed to clear the DMA buffers!" );   // Clear the DMA buffers

	// Enable the ADC I2S mode
	ERROR( ESP_OK != i2s_adc_enable( m_I2SPort ), "Failed to enable ADC I2S Mode!" );

	// Start a task to read samples from I2S
	xTaskCreatePinnedToCore( I2SReaderTask_DAC, "Reader Task", 4096, this, 1, &m_I2STaskHandle, 0 );
}

void	I2SInput::StartI2S( i2s_port_t _I2SPort, U32 _samplingRate, U32 _bufferSize, const i2s_pin_config_t& _pins ) {
	Start( _I2SPort, _samplingRate, _bufferSize, false );

	// Start
	ERROR( ESP_OK != i2s_set_pin( m_I2SPort, &_pins ), "Failed to setup the I2S external pins!" );
	ERROR( ESP_OK != i2s_zero_dma_buffer( m_I2SPort ), "Failed to clear the DMA buffers!" );   // Clear the DMA buffers

	// Start a task to read samples from I2S
	xTaskCreatePinnedToCore( I2SReaderTask_I2S, "Reader Task", 4096, this, 1, &m_I2STaskHandle, 0 );
}

U32	I2SInput::GetSamples( Sample* _samples, U32 _samplesCount ) {
	ERROR( _samplesCount > m_bufferSize, "Buffer is too small to provide the requested amount of samples!" );

	_samplesCount = min( m_sampleIndexWrite - m_sampleIndexRead, _samplesCount );

	S16*	targetSample = (S16*) _samples;
	U32		bufferSampleIndex = m_sampleIndexRead % m_bufferSize;
	U32		samplesCountToEnd = m_bufferSize - bufferSampleIndex;
	if ( _samplesCount <= samplesCountToEnd ) {	// Copy in a single shot
		memcpy( targetSample, m_buffer + bufferSampleIndex, _samplesCount * sizeof(S16) );
	} else {	// Copy in 2 shots
		memcpy( targetSample, m_buffer + bufferSampleIndex, samplesCountToEnd * sizeof(S16) );
		memcpy( targetSample + samplesCountToEnd, m_buffer, (_samplesCount - samplesCountToEnd) * sizeof(S16) );
	}

	return _samplesCount;
}
