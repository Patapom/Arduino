
#include <Arduino.h>
#include "driver/i2s.h"
#include <math.h>
#include <SPIFFS.h>
#include <FS.h>

#include "I2SOutput.h"

// number of frames to try and send at once (a frame is a left and right sample)
#define NUM_FRAMES_TO_SEND 512

void i2sWriterTask( void* param ) {
    I2SOutput*  output = (I2SOutput*) param;
    Frame_t*    frames = (Frame_t*) malloc( NUM_FRAMES_TO_SEND * sizeof(Frame_t) ); // 2kB
    int         availableBytes = 0;
    uint8_t*    buffer = NULL;

    while ( true ) {
        // wait for some data to be requested
        i2s_event_t evt;
        if ( xQueueReceive(output->m_i2sQueue, &evt, portMAX_DELAY) != pdPASS )
            continue;
        if ( evt.type != I2S_EVENT_TX_DONE )
            continue;

        size_t bytesWritten = 0;
        do {
            if ( availableBytes == 0 ) {
                output->m_sample_generator->getFrames( frames, NUM_FRAMES_TO_SEND );    // get some frames from the wave file - a frame consists of a 16 bit left and right sample
                availableBytes = NUM_FRAMES_TO_SEND * sizeof(uint32_t);                 // how many bytes do we now have to send
                buffer = (uint8_t*) frames;
            }

            // Write data to the I2S peripheral
            i2s_write( output->m_i2sPort, buffer, availableBytes, &bytesWritten, portMAX_DELAY );
            availableBytes -= bytesWritten;
            buffer += bytesWritten;

        } while ( bytesWritten > 0 );
    }
}

void I2SOutput::start( i2s_port_t i2sPort, const i2s_pin_config_t& i2sPins, SampleSource& sample_generator ) {
    m_sample_generator = &sample_generator;

    // i2s config for writing both channels of I2S
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = m_sample_generator->sampleRate(),
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
//        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S), Obsolete
        .communication_format = i2s_comm_format_t( I2S_COMM_FORMAT_STAND_I2S ),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024};

    m_i2sPort = i2sPort;

    i2s_driver_install( m_i2sPort, &i2sConfig, 4, &m_i2sQueue );    //install and start i2s driver
    i2s_set_pin( m_i2sPort, &i2sPins ); // set up the i2s pins
    i2s_zero_dma_buffer( m_i2sPort );   // clear the DMA buffers

    // start a task to write samples to the i2s peripheral
    TaskHandle_t writerTaskHandle;
    xTaskCreate( i2sWriterTask, "i2s Writer Task", 4096, this, 1, &writerTaskHandle );
}
