#pragma once

//#define DEBUG_LOG

// Definition for the 16-bits stereo sample
struct Sample {
	S16	left;
	S16	right;
};

// Base interface for our sample generators
class ISampleSource {
public:
    virtual U32		GetSamplingRate() = 0;
    virtual void	GetSamples( Sample* _samples, U32 _samplesCount ) = 0;	// This should fill the samples buffer with the specified number of samples
};

// The base Audio Buffer class that offers the basic functionality of a ring buffer that is fed by samples written by a source and read by a player
// Examples of usage of audio buffers are:
//	• A sample file, e.g. a WAV file => Samples are transfered from the file into the buffer
//	• A radio source => Samples are received by a radio module
//	• A microphone => Samples are received from a microphone
//
class AudioBuffer : public ISampleSource {
public:

	ISampleSource*	m_sampleSource;

	// Buffer for pre-loaded samples
	U32				m_bufferSize;
	Sample*			m_buffer;
	bool			m_autoPreLoad;

	// Cursors indicating which sample we're currently playing and how many are available in the buffer
	U32				m_sampleIndex;				// Warning: the sample index is in [0,+infinity]. The actual index of the sample within the buffer is given by m_sampleIndex % m_samplesCount
	U32				m_preloadedSamplesCount;	// Warning: the samples count is in [0,+infinity]. The actual count of samples within the buffer is given by m_preloadedSamplesCount % m_samplesCount

	#ifdef DEBUG_LOG
		// Debugging
		U32				m_log[256];
		U8				m_logIndex = 0;
		U32				m_starvationEventsCount = 0;	// Counts how many starvation events have occurred since the log was last published...
	#endif

	// Thread safety
//	SemaphoreHandle_t	m_semaphore;	// Semaphores are non-blocking, we don't want that
//	portMUX_TYPE	m_spinLock;	// This asserts continuously...

public:
	AudioBuffer();
	~AudioBuffer();

	// _source, specifies the actual source we're buffering
	// _samplesCount, specifies the size of the buffer, in samples
	// Warning: _autoPreLoad must be set to false if the sampler is used within a high-priority thread and the sampler source is using SPIFFS!
	//				=> You should manually call UpdateBuffer() in the main thread...
	bool    Init( ISampleSource& _source, U32 _samplesCount, bool _autoPreLoad=true );

	// Call this on the main thread to manually fill the buffer with pre-loaded file content (only use if _autoPreLoad = false)
	// Warning: causes an exception if called from a high-priority thread and the sample source is using SPIFFS!!
	// Returns true if an update was made
	bool	UpdateBuffer( U32 _requestedSamplesCount );

	// ISampleSource implementation
	virtual U32		GetSamplingRate() override { return m_sampleSource->GetSamplingRate(); }
	virtual void	GetSamples( Sample* _samples, U32 _samplesCount ) override;
};
