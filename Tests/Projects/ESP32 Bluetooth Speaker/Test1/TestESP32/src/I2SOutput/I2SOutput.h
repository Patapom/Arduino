#pragma once

#ifndef __sampler_base_h__
#define __sampler_base_h__

#include <Arduino.h>
#include "driver/i2s.h"

typedef struct {
    int16_t left;
    int16_t right;
} Frame_t;

// Base class for our sample generators
//
class SampleSource
{
public:
    virtual uint32_t sampleRate() = 0;
    // This should fill the samples buffer with the specified number of frames
    // A frame contains a LEFT and a RIGHT sample. Each sample should be signed 16 bits
    virtual void getFrames(Frame_t *frames, int number_frames) = 0;
};

// Base Class for both the ADC and I2S sampler
//
class I2SOutput
{
private:
    TaskHandle_t    m_i2sWriterTaskHandle;  // I2S write task
    QueueHandle_t   m_i2sQueue;             // i2s writer queue
    i2s_port_t      m_i2sPort;              // i2s port
    SampleSource*   m_sample_generator;     // src of samples for us to play

public:
    void start( i2s_port_t i2sPort, const i2s_pin_config_t& i2sPins, SampleSource& sample_generator );

    friend void i2sWriterTask(void *param);
};

#endif