////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Peripheral Device using ESP-Now
// The Peripheral has a microphone that streams voice to the Central at 16KHz and 2 stereo speakers that receive hi-fi music from the Central at 44.1KHz
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//


//https://www.pschatzmann.ch/home/2022/04/25/low-latency-streaming-of-audio-data/
//It is thus far easier to send the data as fast as possible and just block on the receiving system when the buffer is full.
//This approach works well with all recorded data or data which is created via DSP algorithms. This is by the way also how A2DP works!


#if defined(BUILD_PERIPHERAL)

#include "./Common/Global.h"
#include "./Common/AudioBuffer.h"
#include "./Common/AudioTransformers.h"
#include "./Common/I2S/I2SOutput.h"
#include "./Common/I2S/I2SInput.h"
#include "./Common/WAVFileSampler.h"
#include "./Common/ESPNow/TransportESPNow.h"

#include "Local.h"

#define	USE_WAV	// Define this to use the WAV file as I2S sample source, undefine to use audio packets received via ESP-Now

#define USE_MIC_MEMS	// Define this to use the INMP441 MEMS I2S digital microphone, disable to use the MAX9814 analog microphone

#define FEEDBACK_MICROPHONE	// Use the mic as input, just for testing...


char  str::ms_globalBuffer[256];
char* str::ms_globalPointer = str::ms_globalBuffer;


//////////////////////////////////////////////////////////////
#define PIN_I2S_OUTPUT_DOUT	GPIO_NUM_22  // Data out
#define PIN_I2S_OUTPUT_BCLK	GPIO_NUM_26  // Bits clock
#define PIN_I2S_OUTPUT_LRC	GPIO_NUM_25  // L/R Clock (Word Select)

#define PIN_I2S_INPUT_DIN	GPIO_NUM_21  // Data in
#define PIN_I2S_INPUT_BCLK	GPIO_NUM_18  // Bits clock
#define PIN_I2S_INPUT_LRC	GPIO_NUM_19  // L/R Clock (Word Select)

#define PIN_ADC_VOLUME	34	// Pin D34 (ADC1_CHANNEL_6)

I2SInput	input;	// Microphone
I2SOutput	output;	// Speakers

#ifdef	USE_WAV

WAVFileSampler  WAV;
AudioBuffer		WAVBuffer;	// We must insert a buffer between the WAV file and the ESP-Now uploader to avoid reading directly from the SPIFFS

void	AudioBufferUpdaterTask( void* _param ) {
	while ( true ) {
		WAVBuffer.UpdateBuffer( I2SOutput::NUM_SAMPLES_TO_SEND );	// Update with some samples ASAP
	}
}

ISampleSource&	InitSampleSource() {
	ERROR( !WAV.Init( "/Alarm03.wav" ), "An error occurred while reading the WAV file for I2S source..." );

	#if 0	// Auto-update
		ERROR( !WAVBuffer.Init( WAV, 512 ), "An error occurred while initializing samples buffer!" );
	#elif 1	// Manual update in the main loop
		// Buffer must be 1 kSamples long or stuttering occurs!
		#define UPDATE_IN_MAIN_LOOP
		ERROR( !WAVBuffer.Init( WAV, 4 * I2SOutput::NUM_SAMPLES_TO_SEND, false ), "An error occurred while initializing samples buffer!" );
	#else	// Manual update in a low-priority task
		// Buffer must be 1 kSamples long or stuttering occurs! Even with a high-priority task...
		ERROR( !WAVBuffer.Init( WAV, 4 * I2SOutput::NUM_SAMPLES_TO_SEND, false ), "An error occurred while initializing samples buffer!" );
		TaskHandle_t writerTaskHandle;
		xTaskCreate( AudioBufferUpdaterTask, "AudioBufferUpdaterTask", 4096, NULL, tskIDLE_PRIORITY, &writerTaskHandle );
//		xTaskCreate( AudioBufferUpdaterTask, "AudioBufferUpdaterTask", 4096, NULL, 3, &writerTaskHandle );	// Higher than I2SOutput writer task..
	#endif

	return WAVBuffer;
}

#else

TransportESPNow_Receiver	transportFromCentral;

/*void	OnPacketReceived( U32 _formerPacketID ) {

#if 1	// Simulate a 1KHz sine wave => Even though the sound should be crystal clear, the fact that we only provide new data with an unpredictable clock driven by the rate of received packets makes the sound pop and glitch! :/
static U32		s_sampleIndex = 0;
static Sample	samples[TransportESPNow_Base::SAMPLES_PER_PACKET];
for ( U32 i=0; i < TransportESPNow_Base::SAMPLES_PER_PACKET; i++ ) {
	S16	value = S16( 64 * sin( 2*PI * (1000.0 / 22050) * s_sampleIndex++ ) );
	samples[i].left = value;
	samples[i].right = value;
}
U32	count = output.WriteSamples( samples, TransportESPNow_Base::SAMPLES_PER_PACKET );
if ( count != TransportESPNow_Base::SAMPLES_PER_PACKET ) {
	Serial.printf( "Only %d/%d samples were written!\n", count, TransportESPNow_Base::SAMPLES_PER_PACKET );
}
return;

#endif

	U32	samplesCount = TransportESPNow_Base::SAMPLES_PER_PACKET;
	U32	bufferSampleIndex = (transportFromCentral.m_sampleIndexWrite + transportFromCentral.m_bufferSize - samplesCount) % transportFromCentral.m_bufferSize;
	U32	samplesCountToEnd = min( transportFromCentral.m_bufferSize - bufferSampleIndex, samplesCount );
	U32	writtenSamplesCount = 0;
	if ( samplesCount < samplesCountToEnd ) {
		writtenSamplesCount += output.WriteSamples( transportFromCentral.m_buffer + bufferSampleIndex, samplesCount );
	} else {
		writtenSamplesCount += output.WriteSamples( transportFromCentral.m_buffer + bufferSampleIndex, samplesCountToEnd );
		writtenSamplesCount += output.WriteSamples( transportFromCentral.m_buffer, samplesCount - samplesCountToEnd );
	}
	if ( writtenSamplesCount != samplesCount ) {
		Serial.printf( "Only %d/%d samples were written!\n", writtenSamplesCount, samplesCount );
	}
}
//*/

#ifdef FEEDBACK_MICROPHONE
AudioBuffer						bufferMic;
TransformInterpolateMono2Stereo	transformMic;
#endif

ISampleSource&	InitSampleSource() {
#ifdef FEEDBACK_MICROPHONE
input.m_sampleIndexWrite += 2048;
return input;

	bufferMic.Init( input, 2048, true );
return bufferMic;

	transformMic.Init( bufferMic );
	transformMic.SetTargetSamplingRate( 22050 );

	Serial.println( "Microphone Feedback Initialized..." );

	return transformMic;
#else

	// Initialize the audio buffer
//	transportFromCentral.SetSamplingRate( 44100 );
	transportFromCentral.SetSamplingRate( 22050 );


transportFromCentral.SetSamplingRate( 22000 );	// Reduce a little to give a chance to the receiver to fill its buffer...


//	transportFromCentral.Init( ESP_NOW_WIFI_CHANNEL_CENTRAL_TO_PERIPHERAL, 0x01, 2048 );	// Use receiver ID 1
	transportFromCentral.Init( ESP_NOW_WIFI_CHANNEL_CENTRAL_TO_PERIPHERAL, 0x01, 200 * TransportESPNow_Base::SAMPLES_PER_PACKET );	// Use receiver ID 1

	Serial.println( "Transport from Central Initialized..." );

	return transportFromCentral;
#endif
}

#endif

void  InitSpeakers() {
	// Initialize sample source (either a local WAV file, or a stream through ESP-Now/WiFi)
	ISampleSource&	sampleSource = InitSampleSource();

	i2s_pin_config_t  I2SPins = {
		.bck_io_num = PIN_I2S_OUTPUT_BCLK,
		.ws_io_num = PIN_I2S_OUTPUT_LRC,
		.data_out_num = PIN_I2S_OUTPUT_DOUT,
		.data_in_num = -1
	};

//	output.SetVolume( 0.25f );
//	output.SetVolume( 0.125f );
	output.SetVolume( 0.03125f );
//	output.SetVolume( 0.01f );	// When using mic feedback loop!!!

	output.Start( I2S_NUM_1, I2SPins, sampleSource );

// Using this to directly write to I2S gives scratches...
//transportFromCentral.SetOnPacketReceivedCallback( &OnPacketReceived );

	Serial.println( "I2S Speakers Initialized..." );
}

TransportESPNow_Transmitter	transportToCentral;

//U32		timerCounter = 0;
bool	sendPacket = false;

void IRAM_ATTR	Timer0_ISR() {
//	timerCounter++;
	sendPacket = true;
}

void	InitTransportToCentral() {
	transportToCentral.Init( ESP_NOW_WIFI_CHANNEL_PERIPHERAL_TO_CENTRAL );

	// We're targeting a 8000 Hz stereo samples (= 16 KHz frequency for mono samples) but we can only send 61 stereo samples per packet, so we need a timer working a frequency of 8000/61 = 131.1475 Hz
	// We achieve this using a base frequency of 80 KHz (80MHz / 1000) and counting up to 610, giving us the exact frequency we're looking for
	hw_timer_t*	timer0Config = timerBegin( 0, 1000, true );	// 80MHz / 1000 = 80KHz base frequency
	timerAlarmWrite( timer0Config, 610, true );				// Count up to 610 => 131.1475 Hz * 61 samples = 8000 Hz
	timerAttachInterrupt( timer0Config, &Timer0_ISR, true );
	timerAlarmEnable( timer0Config );

	Serial.println( "Transport to Central Initialized..." );
}

void	InitMicrophone() {
//	pinMode( PIN_ADC, INPUT );

#ifdef USE_MIC_MEMS

	i2s_pin_config_t  I2SPins = {
		.bck_io_num = PIN_I2S_INPUT_BCLK,
		.ws_io_num = PIN_I2S_INPUT_LRC,
		.data_out_num = -1,
		.data_in_num = PIN_I2S_INPUT_DIN,
	};

	input.StartI2S( I2S_NUM_0, 
//					16000,	// 16KHz is enough for voice quality (human voice only reaches up to 8KHz)
22050,	// When testing feedback loop, use the same frequency as the player
					2048,	// Allocate 2048 samples
					I2SPins
					);
//// Which channel is the I2S microphone on? I2S_CHANNEL_FMT_ONLY_LEFT or I2S_CHANNEL_FMT_ONLY_RIGHT
//// Generally they will default to LEFT - but you may need to attach the L/R pin to GND
//#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
//// #define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT

#else
	input.StartADC( I2S_NUM_0, ADC1_CHANNEL_7,
//				 16000,	// 16KHz is enough for voice quality (human voice only reaches up to 8KHz)
44100,	// When testing feedback loop
//22050,
				 2048	// Allocate 2048 samples
				);
#endif

	Serial.println( "I2S Microphone Initialized..." );

#ifndef FEEDBACK_MICROPHONE
	InitTransportToCentral();
#endif
}

void setup() {
	Serial.begin( 115200 );
	Serial.println( "Serial ready" );

	pinMode( PIN_ADC_VOLUME, INPUT );

	if ( !SPIFFS.begin( true ) ) {
		Serial.println( "An Error has occurred while mounting SPIFFS!" );
		return;
	}

	// Initialize WiFi
	WiFi.mode( WIFI_STA );
	WiFi.disconnect();	// We're using ESP-Now
	Serial.println( str( "MAC Address is: %s", WiFi.macAddress().c_str() ) );

	InitMicrophone();
	InitSpeakers();
}

void loop() {
	#ifdef UPDATE_IN_MAIN_LOOP
		WAVBuffer.UpdateBuffer( I2SOutput::NUM_SAMPLES_TO_SEND );	// Update with samples ASAP
	#else

// We can't do that while I2S is enabled and we can't use ADC2 because it's used by WiFi...
//		// Read ADC and convert into volume
//		U16	value = analogRead( PIN_ADC_VOLUME );
//Serial.println( value );
//	//	output.SetVolume( value / 4095.f );


//	  	delay( 1000 );  // to prevent watchdog in release > 1.0.6

	if ( sendPacket ) {
//		U32	packetID = timerCounter;							// Send an audio packet to the central (ID 0)
		U32	packetID = transportToCentral.m_sentPacketsCount;	// Use the transport's packet counter as packet ID, because the timer counter may have changed since it asked us to send the packet
		transportToCentral.SendPacket( input, packetID, 0 );
		sendPacket = false;
	}


//* Show some packet stats
static U32	lastReceivedPacketsCount = 0;
static U32	lastSentPacketsCount = 0;
static U32	lastLostPacketsCount = 0;
static U32	lastReceivedSamplesCount = 0;
static U32	lastTime = millis();

U32	now = millis();
U32	elapsedTime_ms = now - lastTime;
if ( elapsedTime_ms > 1000 ) {
	U32	receivedPackets = transportFromCentral.m_receivedPacketsCount - lastReceivedPacketsCount;
	U32	sentPackets = transportToCentral.m_sentPacketsCount - lastSentPacketsCount;
	U32	lostPackets = transportFromCentral.m_lostPacketsCount - lastLostPacketsCount;
	U32	receivedSamplesCount = input.m_sampleIndexWrite - lastReceivedSamplesCount;

	// Debug ESP-Now packets status
//	Serial.printf( "Received %d, ID=0x%06X = %.1f Hz - Lost = %2d pcks (%.1f%%) | Sent %d, ID=0x%06x = %.1f Hz\n", receivedPackets, transportFromCentral.m_lastReceivedPacketID, TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * receivedPackets) / elapsedTime_ms, lostPackets, 100.0f * lostPackets / receivedPackets, sentPackets, transportToCentral.m_sentPacketsCount-1, 2*TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * sentPackets) / elapsedTime_ms );

	// Debug microphone input levels and sampling frequency
	Serial.printf( "Received %d samples from microphone [%d, %d] - Frequency = %.1f Hz\n", input.m_sampleIndexWrite, input.m_sampleMin, input.m_sampleMax, (1000.0f * receivedSamplesCount) / (now - lastTime) );

	input.m_sampleMin = 0;
	input.m_sampleMax = 0;

	lastReceivedPacketsCount = transportFromCentral.m_receivedPacketsCount;
	lastSentPacketsCount = transportToCentral.m_sentPacketsCount;
	lastLostPacketsCount = transportFromCentral.m_lostPacketsCount;
	lastReceivedSamplesCount = input.m_sampleIndexWrite;
	lastTime = now;
}
//*/

	#endif
}

#endif
