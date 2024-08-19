#pragma once

//#define DEBUG_LOG

class AudioBuffer;

// Definition for the 16-bits stereo sample
// For mono sources, this structure is used as a pair of mono samples and the right channel contains the second sample in the pair
struct Sample {
	S16	left;
	S16	right;
};

// Base interface for our sample generators
class ISampleSource {
public:
	enum CHANNELS {
		MONO = 1,
		STEREO = 2,
	};

public:
    virtual U32			GetSamplingRate() const = 0;
	virtual CHANNELS	GetChannelsCount() const = 0;
    virtual U32			GetSamples( Sample* _samples, U32 _samplesCount ) = 0;	// This should fill the samples buffer with the specified number of samples, returns the actual amount of samples that could be written
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
	U32				m_sampleIndexRead;	// Warning: the sample index is in [0,+infinity]. The actual index of the sample within the buffer is given by m_sampleIndex % m_samplesCount
	U32				m_sampleIndexWrite;	// Warning: the samples count is in [0,+infinity]. The actual count of samples within the buffer is given by m_preloadedSamplesCount % m_samplesCount
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
	// Returns the amount of samples that were actually updated
	U32		UpdateBuffer( U32 _requestedSamplesCount );

	// ISampleSource implementation
	virtual U32			GetSamplingRate() const override { return m_sampleSource->GetSamplingRate(); }
	virtual CHANNELS	GetChannelsCount() const override { return m_sampleSource->GetChannelsCount(); }
	virtual U32			GetSamples( Sample* _samples, U32 _samplesCount ) override;

public:
	// Copy samples from a source circular buffer to a target linear buffer
	static void	CopyFromCircularBuffer( const Sample* _sourceCircularBuffer, U32 _circularBufferSize, U32& _sourceIndex, Sample* _targetBuffer, U32 _samplesCount );
	// Copy samples from a source linear buffer to a target circular buffer
	static void	CopyToCircularBuffer( const Sample* _sourceBuffer, Sample* _targetCircularBuffer, U32 _circularBufferSize, U32& _targetIndex, U32 _samplesCount );

};
