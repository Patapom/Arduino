#pragma once

//#define TIME_USE_XTAL

//#define DEBUG_LOG

class AudioBuffer;

// Definition for the 16-bits stereo sample
// For mono sources, this structure is used as a pair of mono samples and the right channel contains the second sample in the pair
struct Sample {
	S16	left;
	S16	right;
};

// Base interface for the time reference object (i.e. a master player)
class ITimeReference {
public:
	// Tells if the time has started
	virtual bool	HasStarted() const = 0;

	// Start the time
	virtual void	Start() = 0;

	// Gets the current time (in micro-seconds) of the master reference
	// The time starts from 0 when the master reference is starting
	virtual U64		GetTimeMicros() const = 0;
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

	// Samples the source signal at the specified time with the specified sampling rate
	virtual void		GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount ) = 0;
};

// Default reference time implementation
class DefaultTime : public ITimeReference {
public:
	// Master time
	#ifdef TIME_USE_XTAL
		U64			m_XTalCountStart;
		mutable U64	m_XTalCountLast = 0;
	#else
		U64			m_timeStart = 0.0f;
	#endif

public:
	// ITimeReference Implementation
	virtual void	Start() override;
	virtual bool 	HasStarted() const override { return m_timeStart != 0.0f; }
	virtual U64		GetTimeMicros() const override;
};

// The base Audio Buffer class that offers the basic functionality of a ring buffer that is fed by samples written by a source and read by a player
// Examples of usage of audio buffers are:
//	• A sample file, e.g. a WAV file => Samples are transfered from the file into the buffer
//	• A radio source => Samples are received by a radio module
//	• A microphone => Samples are received from a microphone
//
class AudioBuffer : public ISampleSource {
public:

	// The time reference used to read/write samples
	const ITimeReference*	m_time = NULL;

	// Buffer for pre-loaded samples
	U64			m_preloadDelay_Micros = 0;
	U32			m_bufferSize = 0;
	Sample*		m_buffer = NULL;

	// Cursors indicating which sample we're currently reading/writing
	U64			m_sampleIndexRead = 0.0f;			// Warning: the sample index is in [0,+infinity]. The actual index of the sample within the buffer is given by m_sampleIndexRead % m_samplesCount
	float		m_sampleIndexReadDecimal = 0.0f;	// Decimal part
	U64			m_timeRead = 0;						// The time at which samples were last read

	U64			m_sampleIndexWrite = 0;				// Warning: the samples count is in [0,+infinity]. The actual count of samples within the buffer is given by m_sampleIndexWrite % m_samplesCount
	U64			m_timeWrite = 0;					// The time at which samples were last written

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
	AudioBuffer( const ITimeReference& _time );
	~AudioBuffer();

	// ISampleSource implementation
//	virtual U32			GetSamplingRate() const override = 0;
//	virtual CHANNELS	GetChannelsCount() const = 0;
	virtual void		GetSamples( float _samplingRate, Sample* _samples, U32 _samplesCount ) override;

protected:
	//	_bufferSize, the size of the buffer, in samples
	//	_sampleIndexWrite, initial index of the last written sample in the buffer (should be initialized to _preLoadDelay_Seconds * SamplingRate)
	//	_preloadDelay_Micros, how much time delay (in micro-seconds) should the buffer have ahead of the reference read time?
	bool    Init( U32 _bufferSize, U64 _sampleIndexWrite, U64 _preloadDelay_Micros );

	// Feed new samples to the buffer
	void	WriteSamples( const Sample* _samples, U32 _samplesCount );
};
