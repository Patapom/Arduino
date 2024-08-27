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

private:
	// Volume & auto-gain computation
	bool			m_autoGain = true;
	U32				m_autoGainMin = 1;
	U32				m_autoGainMax = 64;
	U32				m_gainFactor = 32;
	U32				m_gainDivider = 2048;

public:
	U32				m_autoGainSumVolume = 0;
	U32				m_autoGainPacketsCount = 0;
	U32				m_autoGainAverageVolume = 0;

public:
	U32				m_volume = 0;
	U32				m_sumVolume = 0;	// Sum of volumes over time, you have to average this over the amount of audio packets stored below
	U32				m_audioPacketsCount = 0;
	U32				m_silentSurroundCounter = 0;

	// Maximum amplitude of received samples (for debug purpose)
	S16				m_sampleMin = 0;
	S16				m_sampleMax = 0;

public:
	I2SInput( const ITimeReference& _time );

	// Starts input sampling the ADC
	bool	StartADC( i2s_port_t _I2SPort, adc1_channel_t _ADCChannel, U32 _samplingRate, float _preLoadDelay );

	// Starts input sampling an I2S microphone
	bool	StartI2S( i2s_port_t _I2SPort, const i2s_pin_config_t& _pins, U32 _samplingRate, float _preLoadDelay );

	// Sets the gain factor (only works for I2S mode, not ADC)
	void	SetGainFactor( U32 _gainFactor ) { m_gainFactor = _gainFactor; m_gainDivider = 65536 / m_gainFactor; m_autoGain = false; }
	U32		GetGainFactor() const { return m_gainFactor; }

	// Enables auto-gain
	//	_min, _max, the min and max factors to apply to the microphone input
	void	EnableAutoGain( U32 _min=1, U32 _max=64 ) { m_autoGain = true; m_autoGainMin = max( 1UL, _min ); m_autoGainMax = _max; }

	// Tells if there is enough sound around to consider the environment as active
	// You can use this and average it over time to consider going to sleep if no sound is actively registering for more than a given amount of time
	// The counter is basically incremented every DMA event, so we can deduce the silent time by using m_samplingRate / DMA_SIZE as the counter for 1s of silence
	U32		GetSilentSurroundCounter() const { return m_silentSurroundCounter; }

	// ISampleSource Implementation
	virtual U32			GetSamplingRate() const override { return m_samplingRate; }
	virtual CHANNELS	GetChannelsCount() const override { return CHANNELS::MONO; }

private:
	bool	Start( i2s_port_t _I2SPort, bool _ADCMode, U32 _samplingRate, float _preLoadDelay );

	void	AutoGain();

	friend void	I2SReaderTask_DAC( void* _param );
	friend void	I2SReaderTask_I2S( void* _param );
};
