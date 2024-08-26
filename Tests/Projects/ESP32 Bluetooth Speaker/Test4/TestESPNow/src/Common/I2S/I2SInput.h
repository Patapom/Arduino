#pragma once

#include "driver/i2s.h"
#include "../AudioBuffer.h"

// Microphone sampler using the I2S unit
// Source: https://www.atomic14.com/2020/09/12/esp32-audio-input.html
//
class I2SInput : public AudioBuffer {
public:

	static const U32	DMA_SIZE = 256;	// Size of the DMA buffer (and consequently, the amount of samples requested by the I2S RX events)

private:
	TaskHandle_t	m_I2STaskHandle;	// I2S reader task
	QueueHandle_t	m_I2SQueue;			// I2S queue
	i2s_port_t		m_I2SPort;			// I2S port

	U32 			m_samplingRate = 0;

	U32				m_gainLog2 = 5;		// Default x32 gain for INMP411, okay for voice but saturates for claps or other loud sounds humans can make

public:
	// Maximum amplitude of received samples (for debug purpose)
	S16				m_sampleMin = 0;
	S16				m_sampleMax = 0;

public:
	I2SInput( const ITimeReference& _time );

	// Starts input sampling the ADC
	bool	StartADC( i2s_port_t _I2SPort, adc1_channel_t _ADCChannel, U32 _samplingRate, float _preLoadDelay );

	// Starts input sampling an I2S microphone
	bool	StartI2S( i2s_port_t _I2SPort, const i2s_pin_config_t& _pins, U32 _samplingRate, float _preLoadDelay );

	// Sets the gain, in bit shifts (only works for I2S mode, not ADC)
	void	SetGainLog2( U32 _gainLog2 ) { m_gainLog2 = _gainLog2; }

	// ISampleSource Implementation
	virtual U32			GetSamplingRate() const override { return m_samplingRate; }
	virtual CHANNELS	GetChannelsCount() const override { return CHANNELS::MONO; }

private:
	bool	Start( i2s_port_t _I2SPort, bool _ADCMode, U32 _samplingRate, float _preLoadDelay );

	friend void	I2SReaderTask_DAC( void* _param );
	friend void	I2SReaderTask_I2S( void* _param );
};
