#pragma once

#include "driver/i2s.h"
#include "../AudioBuffer.h"

// ADC sampler using the I2S unit
// Source: https://www.atomic14.com/2020/09/12/esp32-audio-input.html
//
class I2SInput : public ISampleSource {
public:

	static const U32	ADC_SAMPLES_COUNT = 512;	// Number of samples to buffer
//	static const U32	NUM_SAMPLES_TO_SEND = 64;	// Number of samples to try and send at once

private:
	TaskHandle_t	m_I2STaskHandle;	// I2S reader task
	QueueHandle_t	m_I2SQueue;			// I2S queue
	i2s_port_t		m_I2SPort;			// I2S port

	U32 			m_samplingRate = 0;
	U32 			m_bufferSize = 0;
	S16*			m_buffer = NULL;

	U32				m_gainLog2 = 5;		// Default x32 gain for INMP411, okay for voice but saturates for snaps

public:
	U32				m_sampleIndexWrite = 0;
	U32				m_sampleIndexRead = 0;

	// Maximum amplitude of received samples (for debug purpose)
	S16				m_sampleMin = 0;
	S16				m_sampleMax = 0;

public:
	I2SInput();
	~I2SInput();

	// Starts input sampling the ADC
	// _bufferSize, the size of the buffer (in samples)
	void	StartADC( i2s_port_t _I2SPort, U32 _samplingRate, U32 _bufferSize, adc1_channel_t _ADCChannel );

	// Starts input sampling using an I2S microphone
	// _bufferSize, the size of the buffer (in samples)
	void	StartI2S( i2s_port_t _I2SPort, U32 _samplingRate, U32 _bufferSize, const i2s_pin_config_t& _pins );

	// Sets the gain, in bit shifts (only works for I2S mode, not ADC)
	void	SetGainLog2( U32 _gainLog2 ) { m_gainLog2 = _gainLog2; }

	virtual U32			GetSamplingRate() const override { return m_samplingRate; }
	virtual CHANNELS	GetChannelsCount() const override { return CHANNELS::MONO; }
	virtual U32			GetSamples( Sample* _samples, U32 _samplesCount ) override;

private:
	void	Start( i2s_port_t _I2SPort, U32 _samplingRate, U32 _bufferSize, bool _ADCMode );

	friend void	I2SReaderTask_DAC( void* _param );
	friend void	I2SReaderTask_I2S( void* _param );
};
