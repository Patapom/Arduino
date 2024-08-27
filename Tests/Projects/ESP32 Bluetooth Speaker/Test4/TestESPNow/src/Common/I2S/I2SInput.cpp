#include "../Global.h"
#include "I2SInput.h"

void I2SReaderTask_DAC( void* _param );
void I2SReaderTask_I2S( void* _param );

I2SInput::I2SInput( const ITimeReference& _time )
	: AudioBuffer( _time )
{
}

bool I2SInput::Start( i2s_port_t _I2SPort, bool _ADCMode, U32 _samplingRate, float _preLoadDelay ) {

	// Allocate the audio buffer
	m_samplingRate = _samplingRate;

	U32	preLoadSamplesCount = U32( ceil( _preLoadDelay * m_samplingRate ) );
	U32	bufferSize = preLoadSamplesCount + 8 * DMA_SIZE;	// Give room for 8 updates before we loop back into "read territory"

	if ( !AudioBuffer::Init( bufferSize, preLoadSamplesCount, _preLoadDelay * 1000000.0f ) )
		return false;

	// Initialize I2S Driver
	i2s_config_t i2s_config = {
		.mode = (i2s_mode_t) (_ADCMode ? I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN
									   : I2S_MODE_MASTER | I2S_MODE_RX),

		.sample_rate = m_samplingRate,


// Comment from Atomic14's library
//// Which channel is the I2S microphone on? I2S_CHANNEL_FMT_ONLY_LEFT or I2S_CHANNEL_FMT_ONLY_RIGHT
//// Generally they will default to LEFT - but you may need to attach the L/R pin to GND
//#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
//// #define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT

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
		.dma_buf_len = DMA_SIZE,
		.use_apll = false,
		.tx_desc_auto_clear = false,
		.fixed_mclk = 0
	};

	m_I2SPort = _I2SPort;

	// Install and start I2S driver
	ERROR( ESP_OK != i2s_driver_install( m_I2SPort, &i2s_config, 4, &m_I2SQueue ), "Failed to install I2S driver!" );

	return true;
}

bool I2SInput::StartADC( i2s_port_t _I2SPort, adc1_channel_t _ADCChannel, U32 _samplingRate, float _preLoadDelay ) {
	ERROR( m_I2SPort != I2S_NUM_0, "I2S built-in ADC/DAC only supported on I2S0!" );

	if ( !Start( _I2SPort, true, _samplingRate, _preLoadDelay ) )
		return false;

	// Init ADC pad
	ERROR( ESP_OK != i2s_set_adc_mode( ADC_UNIT_1, _ADCChannel ), "Failed setting ADC mode!" );
	ERROR( ESP_OK != i2s_zero_dma_buffer( m_I2SPort ), "Failed to clear the DMA buffers!" );   // Clear the DMA buffers

	// Enable the ADC I2S mode
	ERROR( ESP_OK != i2s_adc_enable( m_I2SPort ), "Failed to enable ADC I2S Mode!" );

	// Start a task to read samples from I2S
	xTaskCreatePinnedToCore( I2SReaderTask_DAC, "Reader Task", 4096, this, 1, &m_I2STaskHandle, 0 );

	return true;
}

bool	I2SInput::StartI2S( i2s_port_t _I2SPort, const i2s_pin_config_t& _pins, U32 _samplingRate, float _preLoadDelay ) {
	if ( !Start( _I2SPort, false, _samplingRate, _preLoadDelay ) )
		return false;

	// Start
	ERROR( ESP_OK != i2s_set_pin( m_I2SPort, &_pins ), "Failed to setup the I2S external pins!" );
	ERROR( ESP_OK != i2s_zero_dma_buffer( m_I2SPort ), "Failed to clear the DMA buffers!" );   // Clear the DMA buffers

	// Start a task to read samples from I2S
	xTaskCreatePinnedToCore( I2SReaderTask_I2S, "Reader Task", 4096, this, 1, &m_I2STaskHandle, 0 );

	return true;
}

void	I2SInput::AutoGain() {
	if ( !m_autoGain )
		return;

	// We want to quickly attenuate any violent sound and slowly increase the gain to reach an average volume
	const U32	TARGET_VOLUME = 1000000;
	const U32	SILENCE_VOLUME = 80000;
//	const U32	SATURATION_VOLUME = 10000000;	// Working
//	const U32	SATURATION_VOLUME = 4000000;	// Better
	const U32	SATURATION_VOLUME = 2000000;

	// Determine a fast average volume to work with
	m_autoGainSumVolume += m_volume;
	m_autoGainPacketsCount++;
	if ( m_autoGainPacketsCount >= 4 ) {	// 4 * 256 sample packets in the DMA = 1/10s average at 16KHz
		// Compute average and reset sums
		m_autoGainAverageVolume = m_autoGainSumVolume / m_autoGainPacketsCount;
		m_autoGainSumVolume = 0;
		m_autoGainPacketsCount = 0;
	}

//	if ( m_autoGainAverageVolume > SATURATION_VOLUME ) {
	if ( m_volume > SATURATION_VOLUME ) {
		// Decrease gain immediately
		m_gainFactor = m_gainFactor > m_autoGainMin + 4UL ? m_gainFactor - 4UL : m_autoGainMin;
		m_gainDivider = 65536 / m_gainFactor;
		m_autoGainAverageVolume = TARGET_VOLUME;
		m_silentSurroundCounter = 0;

	} else if ( m_autoGainAverageVolume < SILENCE_VOLUME ) {
		// Increase gain slowy using average volume
		#if 1
			m_gainFactor = min( m_autoGainMax, TARGET_VOLUME / max( 1UL, m_autoGainAverageVolume ) );
		#else
			m_gainFactor = min( m_autoGainMax, m_gainFactor + 1UL );
		#endif
		m_gainDivider = 65536 / m_gainFactor;
		m_silentSurroundCounter = 0;
		m_autoGainAverageVolume = TARGET_VOLUME;
	}

	if ( m_gainFactor == m_autoGainMax ) {
		m_silentSurroundCounter++;
	}
}


//////////////////////////////////////////////////////////////////////////////
// Sampling tasks
//////////////////////////////////////////////////////////////////////////////

// I2S DAC sampling reads 12-bits values that need to be expanded to 16-bits
void I2SReaderTask_DAC( void* _param ) {
	S16			tempBuffer[I2SInput::DMA_SIZE];

	I2SInput*	that = (I2SInput*) _param;

	// Wait until the reference time starts
	while ( !that->m_time->HasStarted() ) {
		delay( 1 );
	}

	while ( true ) {
		// Wait for some data to arrive on the queue
		i2s_event_t evt;
		if ( xQueueReceive( that->m_I2SQueue, &evt, portMAX_DELAY) != pdPASS )
			continue;
		if ( evt.type != I2S_EVENT_RX_DONE )
			continue;

		// Read samples
		size_t	bytesRead = 0;
		i2s_read( that->m_I2SPort, tempBuffer, I2SInput::DMA_SIZE * sizeof(U16), &bytesRead, 10 / portTICK_PERIOD_MS );
		if ( bytesRead != I2SInput::DMA_SIZE * sizeof(U16) )
			throw "Couldn't read the expected number of samples!!";

//		U32	readSamplesCount = bytesRead >> 2;
		U32	readSamplesCount = I2SInput::DMA_SIZE;

		// Expand 12-bits unsigned mono samples into 16-bits signed mono samples
		S16*	sample = tempBuffer;
		for ( U32 i=0; i < readSamplesCount; i++ ) {
			S16	value = clamp( *sample, S16(0), S16(4095) );	// In [0,4095]
				value = value - 2048;							// Now in [-2048,2047]
				value *= 16;									// Now in [-32768,32752]
			*sample++ = value;

that->m_sampleMin = min( that->m_sampleMin, value );
that->m_sampleMax = max( that->m_sampleMax, value );
		}

		// Write samples
		that->WriteSamples( (Sample*) tempBuffer, readSamplesCount );
	}
}

// I2S MEMS like the INMP411 send 24-bits signed samples inside a 32-bits word
void I2SReaderTask_I2S( void* _param ) {
	S32			tempBuffer[I2SInput::DMA_SIZE];

	I2SInput*	that = (I2SInput*) _param;

	// Wait until the reference time starts
	while ( !that->m_time->HasStarted() ) {
		delay( 1 );
	}

	while ( true ) {
		// Wait for some data to arrive on the queue
		i2s_event_t evt;
		if ( xQueueReceive( that->m_I2SQueue, &evt, portMAX_DELAY ) != pdPASS )
			continue;
		if ( evt.type != I2S_EVENT_RX_DONE )
			continue;

		// Read samples
		size_t	bytesRead = 0;
		i2s_read( that->m_I2SPort, tempBuffer, I2SInput::DMA_SIZE * sizeof(U32), &bytesRead, 10 / portTICK_PERIOD_MS );
		if ( bytesRead != I2SInput::DMA_SIZE * sizeof(U32) ) {
			throw "Couldn't read the expected number of samples!!";
//Serial.printf( "Unexpected read length %d\n", bytesRead );
//continue;
		}

//		U32	readSamplesCount = bytesRead >> 2;
		U32	readSamplesCount = I2SInput::DMA_SIZE;

		S32	gainDivider = that->m_gainDivider;	// This can change any time so we can't precompute...

		// Compress 24-bits signed mono samples into 16-bits signed mono samples
		S32*	sampleSource = tempBuffer;			// 24-bit signed values [-8388608, 8388607] in the top 3 MSB
		S16*	sampleTarget = (S16*) tempBuffer;	// Write in-place
		S32		temp;
		U32		volume = 0;
		for ( U32 i=0; i < readSamplesCount; i++ ) {
			// Constrain into [-32768,32767]
			temp = *sampleSource++ / gainDivider;
			temp = temp < -32768 ? -32768 : (temp > 32767 ? 32767 : temp);
			*sampleTarget++ = S16( temp );

			volume += U32( temp * temp );
//that->m_sampleMin = min( that->m_sampleMin, sampleTarget[-1] );
//that->m_sampleMax = max( that->m_sampleMax, sampleTarget[-1] );
		}
		volume /= readSamplesCount;	// Average volume
		that->m_volume = volume;
		that->m_sumVolume += volume;
		that->m_audioPacketsCount++;
		that->AutoGain();

#if 0
static U32	s_sampleIndex = 0;
S16*	sample = (S16*) tempBuffer;
U32		samplingRate = that->m_samplingRate;
for ( U32 i=0; i < I2SInput::DMA_SIZE; i++ ) {
	*sample++ = FastSine( s_sampleIndex++ * (16384 * 1000 / samplingRate) ) / 8;
}
#endif

		// Write samples
		that->WriteSamples( (Sample*) tempBuffer, readSamplesCount );
	}
}
