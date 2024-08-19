////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Peripheral Device using ESP-Now
// The Peripheral has a microphone that streams voice to the Central at 16KHz and 2 stereo speakers that receive hi-fi music from the Central at 44.1KHz
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#if defined(BUILD_PERIPHERAL)

#include "./Common/Global.h"
#include "./Common/AudioBuffer.h"
#include "./Common/AudioTransformers.h"
#include "./Common/I2S/I2SOutput.h"
#include "./Common/I2S/I2SInput.h"
#include "./Common/WAVFileSampler.h"
#include "./Common/ESPNow/TransportESPNow.h"

#include "Local.h"

// Reception
#define RECEIVE_AUDIO		// Define this to receive audio (44.1KHz stereo)
//#define	USE_WAV				// Define this to use the WAV file as I2S sample source, undefine to use audio packets received via ESP-Now
//#define FEEDBACK_MICROPHONE	// Use the mic as input, just for testing...

// Transmission
//#define TRANSMIT_AUDIO	// Define this to transmit audio (microphone at 16KHz mono)
#define USE_MIC_MEMS	// Define this to use the INMP441 MEMS I2S digital microphone, disable to use the MAX9814 analog microphone


char  str::ms_globalBuffer[256];
char* str::ms_globalPointer = str::ms_globalBuffer;


//////////////////////////////////////////////////////////////
#define PIN_I2S_OUTPUT_DOUT	GPIO_NUM_22  // Data out
#define PIN_I2S_OUTPUT_BCLK	GPIO_NUM_26  // Bits clock
#define PIN_I2S_OUTPUT_LRC	GPIO_NUM_25  // L/R Clock (Word Select)

#define PIN_I2S_INPUT_DIN	GPIO_NUM_21  // Data in
#define PIN_I2S_INPUT_BCLK	GPIO_NUM_18  // Bits clock
#define PIN_I2S_INPUT_LRC	GPIO_NUM_19  // L/R Clock (Word Select)

//#define PIN_ADC_VOLUME	34	// Pin D34 (ADC1_CHANNEL_6)

#define PIN_BUTTON			GPIO_NUM_5	// Button with pull-up resistor

DefaultTime	mainTime;

// ====================================================================================
// Reception
//
#ifdef RECEIVE_AUDIO

I2SOutput	speakers( mainTime );	// Speakers

#ifdef	USE_WAV
//#define UPDATE_IN_MAIN_LOOP

WAVFileSampler  WAV( mainTime );

ISampleSource&	InitSampleSource() {
	#ifdef UPDATE_IN_MAIN_LOOP 	// Manual update in the main loop
		// Use a long 1 second buffer ahead to compensate irregular timing due to serial print display once a second
		ERROR( !WAV.Init( "/Alarm03.wav", 1.0f ), "An error occurred while reading the WAV file for I2S source..." );
	#else	// Update in a low-priority task
		// Pre-load 0.5s in advance = 0.5 * 22050 = 11025 samples ahead
		ERROR( !WAV.Init( "/Alarm03.wav", 0.5f ), "An error occurred while reading the WAV file for I2S source..." );

//		WAV.StartAutoUpdateTask( tskIDLE_PRIORITY );	// Super wobbly!
		WAV.StartAutoUpdateTask( 3 );					// Higher than I2SOutput writer task..
	#endif

	return WAV;
}

#else // if !defined(USE_WAV)

TransportESPNow_Receiver	transportFromCentral( mainTime );

/*void	OnPacketReceived( U32 _formerPacketID ) {

#if 1	// Simulate a 1KHz sine wave => Even though the sound should be crystal clear, the fact that we only provide new data with an unpredictable clock driven by the rate of received packets makes the sound pop and glitch! :/
static U32		s_sampleIndex = 0;
static Sample	samples[TransportESPNow_Base::SAMPLES_PER_PACKET];
for ( U32 i=0; i < TransportESPNow_Base::SAMPLES_PER_PACKET; i++ ) {
	S16	value = S16( 64 * sin( 2*PI * (1000.0 / 22050) * s_sampleIndex++ ) );
	samples[i].left = value;
	samples[i].right = value;
}
U32	count = speakers.WriteSamples( samples, TransportESPNow_Base::SAMPLES_PER_PACKET );
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
		writtenSamplesCount += speakers.WriteSamples( transportFromCentral.m_buffer + bufferSampleIndex, samplesCount );
	} else {
		writtenSamplesCount += speakers.WriteSamples( transportFromCentral.m_buffer + bufferSampleIndex, samplesCountToEnd );
		writtenSamplesCount += speakers.WriteSamples( transportFromCentral.m_buffer, samplesCount - samplesCountToEnd );
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

#else // !defined(FEEDBACK_MICROPHONE)

	// Initialize the audio buffer
//	transportFromCentral.Init( ESP_NOW_WIFI_CHANNEL_CENTRAL_TO_PERIPHERAL, 0x01, 44100, ISampleSource::STEREO, 0.5f );	// Use receiver ID 1
	transportFromCentral.Init( ESP_NOW_WIFI_CHANNEL_CENTRAL_TO_PERIPHERAL, 0x01, 22050, ISampleSource::STEREO, 0.5f );	// Use receiver ID 1

	Serial.println( "Transport from Central Initialized..." );

	return transportFromCentral;
#endif	// #ifdef FEEDBACK_MICROPHONE
}

#endif	// ifdef USE_WAV

void  InitSpeakers( ISampleSource& _sampleSource, float _volume ) {
	i2s_pin_config_t  I2SPins = {
		.bck_io_num = PIN_I2S_OUTPUT_BCLK,
		.ws_io_num = PIN_I2S_OUTPUT_LRC,
		.data_out_num = PIN_I2S_OUTPUT_DOUT,
		.data_in_num = -1
	};

	speakers.SetVolume( _volume );

	speakers.Init( I2S_NUM_1, I2SPins, _sampleSource, 44100 );

// Using this to directly write to I2S gives scratches...
//transportFromCentral.SetOnPacketReceivedCallback( &OnPacketReceived );

	Serial.println( "I2S Speakers Initialized..." );
}

#endif	// #ifdef RECEIVE_AUDIO

// ====================================================================================
// Reception
//
#ifdef TRANSMIT_AUDIO
TransportESPNow_Transmitter	transportToCentral;
I2SInput	input;	// Microphone

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

#endif // #ifdef TRANSMIT_AUDIO

#include "xtensa/core-macros.h"	// XTHAL_GET_CCOUNT()

void setup() {
	Serial.begin( 115200 );
	Serial.println( "Serial ready" );

//	pinMode( PIN_ADC_VOLUME, INPUT );
	pinMode( GPIO_NUM_5, INPUT );

	if ( !SPIFFS.begin( true ) ) {
		Serial.println( "An Error has occurred while mounting SPIFFS!" );
		return;
	}

	// Initialize WiFi
	WiFi.mode( WIFI_STA );
	WiFi.disconnect();	// We're using ESP-Now
	Serial.println( str( "MAC Address is: %s", WiFi.macAddress().c_str() ) );

#ifdef TRANSMIT_AUDIO
	InitMicrophone();
#endif

#ifdef RECEIVE_AUDIO

	// Initialize sample source (either a local WAV file, or a stream through ESP-Now/WiFi)
	ISampleSource&	sampleSource = InitSampleSource();
	
//	float	volume = 0.25f;
//	float	volume = 0.125f;
	float	volume = 0.03125f;
//	float	volume = 0.01f;	// When using mic feedback loop!!!

	InitSpeakers( sampleSource, volume );

//float	time = speakers.GetTime();
//delay( 1234 );
//float	time2 = speakers.GetTime();
//delay( 2345 );
//float	time3 = speakers.GetTime();
////Serial.printf( "Initial time value => 0x%08X%08X = %f / %f / %f\n", U32( speakers.m_XTalCountLast >> 32ULL ), U32( speakers.m_XTalCountLast ), time, time2, time3 );
//Serial.printf( "Initial time value => %f / %f / %f\n", time, time2, time3 );

#endif

	// Start the show!
	mainTime.Start();
}

void loop() {
//	delay( 1000 );  // to prevent watchdog in release > 1.0.6

	#ifdef TRANSMIT_AUDIO
		if ( sendPacket ) {
//			U32	packetID = timerCounter;							// Send an audio packet to the central (ID 0)
			U32	packetID = transportToCentral.m_sentPacketsCount;	// Use the transport's packet counter as packet ID, because the timer counter may have changed since it asked us to send the packet
			transportToCentral.SendPacket( input, packetID, 0 );
			sendPacket = false;
		}
	#endif

	#if defined(RECEIVE_AUDIO) && defined(USE_WAV)

		#ifdef UPDATE_IN_MAIN_LOOP
			if ( digitalRead( PIN_BUTTON ) )
				WAV.Update( speakers );	// Update with samples ASAP
		#endif

//*
static U32	lastTime = millis();
static U64	lastSampleIndexWrite = 0;
static U64	lastTimeWrite = 0;
static U32	lastUpdateCount = 0;
U32	now = millis();
U32	elapsedTime_ms = now - lastTime;
if ( elapsedTime_ms > 1000 ) {
//	Serial.printf( "%f > Index write = 0x%08X, Time write = %.2f (Sampling Rate = %f Hz) Updates Count = %d/s\n", speakers.GetTime(), WAV.m_sampleIndexWrite, WAV.m_timeWrite, (1000.0f * (WAV.m_sampleIndexWrite - lastSampleIndexWrite)) / (now - lastTime), WAV.m_updatesCount - lastUpdateCount );

	float	sampleIndexRead = WAV.m_sampleIndexRead + WAV.m_sampleIndexReadDecimal;
	float	timeRead = WAV.m_timeRead / 1000000.0f;
	float	samplingRateRead = (WAV.m_sampleIndexWrite - sampleIndexRead) / ((WAV.m_timeWrite - WAV.m_timeRead) / 1000000.0f);

	float	timeWrite = WAV.m_timeWrite / 1000000.0f;
	float	samplingRateWrite0 = (1000000.0f * (WAV.m_sampleIndexWrite - lastSampleIndexWrite)) / (WAV.m_timeWrite - lastTimeWrite);
	float	samplingRateWrite1 = (1000.0f * (WAV.m_sampleIndexWrite - lastSampleIndexWrite)) / (now - lastTime);
	Serial.printf( "%f > Read = %.2f, %.3f (Sampling Rate = %.2f) | Write = %d, %.3f (Sampling Rate = %.2f/%.2f Hz) | Updates Count = %d/s\n", mainTime.GetTimeMicros() / 1000000.0f, sampleIndexRead, timeRead, samplingRateRead, U32(WAV.m_sampleIndexWrite), timeWrite, samplingRateWrite0, samplingRateWrite1, WAV.m_updatesCount - lastUpdateCount );

	lastSampleIndexWrite = WAV.m_sampleIndexWrite;
	lastTimeWrite = WAV.m_timeWrite;
	lastUpdateCount = WAV.m_updatesCount;
	lastTime = now;
}
//*/
	#endif

// We can't do that while I2S is enabled and we can't use ADC2 because it's used by WiFi...
//		// Read ADC and convert into volume
//		U16	value = analogRead( PIN_ADC_VOLUME );
//Serial.println( value );
//	//	speakers.SetVolume( value / 4095.f );

#if (defined(RECEIVE_AUDIO) || defined(TRANSMIT_AUDIO)) && !defined(USE_WAV)
//* Show some packet stats
static U32	lastReceivedPacketsCount = 0;
static U32	lastSentPacketsCount = 0;
static U32	lastLostPacketsCount = 0;
static U32	lastReceivedSamplesCount = 0;
static U32	lastTime = millis();

U32	now = millis();
U32	elapsedTime_ms = now - lastTime;
if ( elapsedTime_ms > 1000 ) {
	#ifdef RECEIVE_AUDIO
		U32	receivedPackets = transportFromCentral.m_receivedPacketsCount - lastReceivedPacketsCount;
		U32	lostPackets = transportFromCentral.m_lostPacketsCount - lastLostPacketsCount;
		U32	lastReceivedPacketID = transportFromCentral.m_lastReceivedPacketID;
	#else
		U32	receivedPackets = 0;
		U32	lostPackets = 0;
		U32	lastReceivedPacketID = 0;
	#endif

	#ifdef TRANSMIT_AUDIO
		U32	sentPackets = transportToCentral.m_sentPacketsCount - lastSentPacketsCount;
		U32	receivedSamplesCount = input.m_sampleIndexWrite - lastReceivedSamplesCount;
		U32	lastSentPacketID = transportToCentral.m_sentPacketsCount-1;
	#else
		U32	sentPackets = 0;
		U32	receivedSamplesCount = 0;
		U32	lastSentPacketID = -1;
	#endif

	// Debug ESP-Now packets status
	Serial.printf( "Received %d, ID=0x%06X = %.1f Hz - Lost = %2d pcks (%.1f%%) | Sent %d, ID=0x%06x = %.1f Hz\n", receivedPackets, lastReceivedPacketID, TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * receivedPackets) / elapsedTime_ms, lostPackets, 100.0f * lostPackets / receivedPackets, sentPackets, lastSentPacketID, 2*TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * sentPackets) / elapsedTime_ms );

	// Debug microphone input levels and sampling frequency
//	Serial.printf( "Received %d samples from microphone [%d, %d] - Frequency = %.1f Hz\n", input.m_sampleIndexWrite, input.m_sampleMin, input.m_sampleMax, (1000.0f * receivedSamplesCount) / (now - lastTime) );

	lastReceivedPacketsCount = transportFromCentral.m_receivedPacketsCount;
	#ifdef RECEIVE_AUDIO
		lastLostPacketsCount = transportFromCentral.m_lostPacketsCount;
	#endif
	#ifdef TRANSMIT_AUDIO
		lastSentPacketsCount = transportToCentral.m_sentPacketsCount;
		lastReceivedSamplesCount = input.m_sampleIndexWrite;

		input.m_sampleMin = 0;
		input.m_sampleMax = 0;
	#endif
	lastTime = now;
}
//*/
#endif

}

#endif	// defined(BUILD_PERIPHERAL)
