////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Peripheral Device using ESP-Now
// The Peripheral has a microphone that streams voice to the Central at 16KHz and 2 stereo speakers that receive hi-fi music from the Central at 44.1KHz
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#if defined(BUILD_PERIPHERAL)

#include "./Common/Global.h"
#include "./Common/AudioBuffer.h"
#include "./Common/I2S/I2SOutput.h"
#include "./Common/I2S/I2SInput.h"
#include "./Common/ESPNow/TransportESPNow.h"
#include "./Common/AudioTools/AudioTransformers.h"
#include "./Common/AudioTools/WaveGenerator.h"
#include "./Common/AudioTools/WAVFileSampler.h"

#include "xtensa/core-macros.h"	// XTHAL_GET_CCOUNT()

#include "Local.h"


//#define VOLUME	0.25f
//#define VOLUME	0.125f
#define VOLUME	0.0625f
//#define VOLUME	0.03125f	// For when using mic feedback loop!!!
//#define VOLUME	0.01f	// For when using mic feedback loop!!!


//#define USE_DUMMY_MICROPHONE	// Use a dummy 1KHz sine wave generator instead of the microphone

// Reception
#define RECEIVE_AUDIO			// Define this to receive audio using ESP-Now, undefine to use either microphone feedback or a WAV sample
//#define FEEDBACK_MICROPHONE		// Use the mic as input, just for testing... Undefine to use a WAV sample

// Transmission
//#define TRANSMIT_AUDIO			// Define this to transmit audio using ESP-Now
//#define TRANSMIT_WAV			// Define this to transmit a WAV sample instead of microphone
#define USE_MIC_MEMS			// Define this to use the INMP441 MEMS I2S digital microphone, disable to use the MAX9814 analog microphone
#define DISABLE_TRANSMIT_ON_SILENCE	4	// Define this to disable transmission of microphone audio packets if the environment is silent for more than the time in seconds defined here

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

I2SOutput	speakers( mainTime );

#ifndef USE_DUMMY_MICROPHONE
I2SInput	microphone( mainTime );
#else
WaveGenerator	microphone;
#endif

// ====================================================================================
// ESP-Now Radio Reception
//
#ifdef RECEIVE_AUDIO

TransportESPNow_Receiver	transportFromCentral( mainTime );

ISampleSource&	InitSampleSource() {
	// Initialize the audio buffer
	transportFromCentral.Init( 0x01, CENTRAL_TO_PERIPHERAL_RATE, ISampleSource::STEREO, 1.0f );	// Use receiver ID 1

	Serial.println( "Transport from Central Initialized..." );

	return transportFromCentral;
}

#elif defined(FEEDBACK_MICROPHONE) // if !defined(RECEIVE_AUDIO)

ISampleSource&	InitSampleSource() {
	return microphone;
}

#else	// if !defined(RECEIVE_AUDIO) && !defined(FEEDBACK_MICROPHONE)

//#define UPDATE_IN_MAIN_LOOP	// Don't! Super slow!

WAVFileSampler  WAV( mainTime );

ISampleSource&	InitSampleSource() {
	#ifdef UPDATE_IN_MAIN_LOOP 	// Manual update in the main loop
		// Use a long 1 second buffer ahead to compensate irregular timing due to serial print display once a second
		ERROR( !WAV.Init( "/Alarm03.wav", 1.0f ), "An error occurred while reading the WAV file for I2S source..." );
	#else	// Update in a low-priority task
		// Pre-load 0.5s in advance = 0.5 * 22050 = 11025 samples ahead
		ERROR( !WAV.Init( "/Alarm03.wav", 0.5f ), "An error occurred while reading the WAV file for I2S source..." );

//WAV.ConvertToMono();

//		WAV.StartAutoUpdateTask( tskIDLE_PRIORITY );	// Super wobbly!
		WAV.StartAutoUpdateTask( 3 );					// Higher than I2SOutput writer task..
	#endif

	return WAV;
}

#endif	// ifdef RECEIVE_AUDIO


// ====================================================================================
// ESP-Now Radio Transmission
//
#ifdef TRANSMIT_AUDIO

TransportESPNow_Transmitter	transportToCentral( mainTime );

#ifdef TRANSMIT_WAV
WAVFileSampler  WAV( mainTime );
#endif

void	InitTransportToCentral() {

	transportToCentral.Init( PERIPHERAL_TO_CENTRAL_RATE );

	// Start transmission task
	U8	receiverMaskID = 0x00;	// The Central has the special receiver ID 0

	#ifndef TRANSMIT_WAV
		#if !defined(RECEIVE_AUDIO) && defined(FEEDBACK_MICROPHONE)
			ERROR! Microphone cannot both be used as feedback into output and by the audio transmitter!
		#endif
		transportToCentral.StartAutoSendTask( 3, microphone, receiverMaskID );
	#else
		// Pre-load 0.5s in advance = 0.5 * 22050 = 11025 samples ahead
		ERROR( !WAV.Init( "/Alarm03.wav", 0.1f ), "An error occurred while reading the WAV file for transmission..." );

		// The microphone channel is MONO, so fake a mono WAV...
		WAV.ConvertToMono();

//		WAV.StartAutoUpdateTask( tskIDLE_PRIORITY );	// Super wobbly!
		WAV.StartAutoUpdateTask( 3 );					// Higher than I2SOutput writer task..

		transportToCentral.StartAutoSendTask( 3, WAV, receiverMaskID );
	#endif

	Serial.println( "Transport to Central Initialized..." );
}

#endif // #ifdef TRANSMIT_AUDIO


// ====================================================================================
// I2S Speakers / Microphone
//
void  InitSpeakers( ISampleSource& _sampleSource, float _volume ) {
	i2s_pin_config_t  I2SPins = {
		.bck_io_num = PIN_I2S_OUTPUT_BCLK,
		.ws_io_num = PIN_I2S_OUTPUT_LRC,
		.data_out_num = PIN_I2S_OUTPUT_DOUT,
		.data_in_num = -1
	};

	speakers.SetVolume( _volume );

	speakers.Init( I2S_NUM_1, I2SPins, _sampleSource, CENTRAL_TO_PERIPHERAL_RATE );

	Serial.println( "I2S Speakers Initialized..." );
}

void	InitMicrophone() {
	#ifndef USE_DUMMY_MICROPHONE
//		pinMode( PIN_ADC, INPUT );

		#ifdef USE_MIC_MEMS

			i2s_pin_config_t  I2SPins = {
				.bck_io_num = PIN_I2S_INPUT_BCLK,
				.ws_io_num = PIN_I2S_INPUT_LRC,
				.data_out_num = -1,
				.data_in_num = PIN_I2S_INPUT_DIN,
			};

			ERROR( !microphone.StartI2S( I2S_NUM_0, I2SPins, PERIPHERAL_TO_CENTRAL_RATE, 0.5f ), "Failed to initalize microphone!" );

		#else
			ERROR( !microphone.StartADC( I2S_NUM_0, ADC1_CHANNEL_7, PERIPHERAL_TO_CENTRAL_RATE, 0.5f ), "Failed to initalize microphone!" );
		#endif

		Serial.println( "I2S Microphone Initialized..." );

	#else
		// Create a dummy 1KHz tone
		microphone.SetChannelsCount( ISampleSource::MONO );
		microphone.SetWaveLeft( 1000.0f, 0.1f );
		microphone.SetWaveRight( 1000.0f, 0.1f );

		Serial.println( "Dummy 1KHz Microphone Initialized..." );

	#endif
}

S16	gs_sine[16384];

void setup() {
	pinMode( PIN_LED_RED, OUTPUT );

//	InitSine();

	Serial.begin( 115200 );
	Serial.println( "Serial ready" );

//	pinMode( PIN_ADC_VOLUME, INPUT );
	pinMode( GPIO_NUM_5, INPUT );

	if ( !SPIFFS.begin( true ) ) {
		Serial.println( "An Error has occurred while mounting SPIFFS!" );
		return;
	}

	// ========================================================================================
	// Initialize WiFi
	TransportESPNow_Base::ConfigureWiFi( ESP_NOW_WIFI_CHANNEL );


	// ========================================================================================
	// Initialize Speakers & Microphone + ESP-Now Transports 
	InitMicrophone();

	#ifdef TRANSMIT_AUDIO
		InitTransportToCentral();
	#endif

	// Initialize sample source (either a stream through ESP-Now/WiFi, or a local WAV file) (or the microphone feedback but careful with that!)
	ISampleSource&	sampleSource = InitSampleSource();

	InitSpeakers( sampleSource, VOLUME );

	// Start the show!
	mainTime.Start();
}

bool	read_button() { return digitalRead( PIN_BUTTON ); }
bool debounce() {
  static uint16_t state = 0;
  state = (state<<1) | read_button() | 0xfe00;
  return (state == 0xff00);
}

// Source: https://hackaday.com/2015/12/09/embed-with-elliot-debounce-your-noisy-buttons-part-i/
enum ButtonStates { UP, DOWN, PRESS, RELEASE };
static ButtonStates	state = UP;
ButtonStates delay_debounce( ButtonStates& button_state ) {        
    if (read_button()){	// if pressed
        if (button_state == PRESS){
            button_state = DOWN;
        } 
        if (button_state == UP){
            delay(5);
            if (read_button()){
                button_state = PRESS;
            }
        } 
    } else {	// if not pressed
        if (button_state == RELEASE){
            button_state = UP;
        } 
        if (button_state == DOWN){
			delay(5);
			if ( !read_button() ){
				button_state = RELEASE;
			}
        }
    }

	return button_state;
}

void loop() {
//	delay( 1000 );  // to prevent watchdog in release > 1.0.6

// We can't do that while I2S is enabled and we can't use ADC2 because it's used by WiFi...
//	// Read ADC and convert into volume
//	U16	value = analogRead( PIN_ADC_VOLUME );
//	speakers.SetVolume( value / 4095.f );
//Serial.println( value );

#if defined(TRANSMIT_AUDIO) && defined(USE_MIC_MEMS) && defined(DISABLE_TRANSMIT_ON_SILENCE)
	// Disable packets transmission if no sound is detected
	U32	silence_seconds = microphone.GetSilentSurroundCounter() * microphone.DMA_SIZE / microphone.GetSamplingRate();
	transportToCentral.m_blockPackets = silence_seconds > DISABLE_TRANSMIT_ON_SILENCE;
#endif

#if 1
	// Toggle packets reception
	static bool		receiveEnabled = true;
	if ( delay_debounce( state ) == RELEASE ) {
		receiveEnabled = !receiveEnabled;
		transportFromCentral.BlockPackets( receiveEnabled );
	}

#elif 1
	// Toggle volume
	static bool		volumeEnabled = true;
	static float	oldVolume = 0;
	if ( delay_debounce( state ) == RELEASE ) {
		volumeEnabled = !volumeEnabled;
		if ( !volumeEnabled ) {
			oldVolume = speakers.GetVolume();
			speakers.SetVolume( 0 );
		} else {
			speakers.SetVolume( oldVolume );
		}
	}
#endif


	#if !defined(RECEIVE_AUDIO) && !defined(FEEDBACK_MICROPHONE)	// Replay WAV sample locally

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

#if 1	// To debug audio buffer read/write indices & interpolation to catch up with loss of packets
	{
		static U32	lastTime = millis();

		U32	now = millis();
		U32	elapsedTime_ms = now - lastTime;
		if ( elapsedTime_ms > 1000 ) {
			float	timeRead = transportFromCentral.m_timeRead / 1000000.0f;
			float	timeWrite = transportFromCentral.m_timeWrite / 1000000.0f;
			float	deltaTime = (transportFromCentral.m_timeWrite - transportFromCentral.m_timeRead) / 1000000.0f;

			U32	deltaRW = transportFromCentral.m_sampleIndexWrite - transportFromCentral.m_sampleIndexRead;
			U32	expectedDeltaRW = transportFromCentral.m_preloadDelay_Micros * transportFromCentral.m_samplingRate / 1000000.0f;

			Serial.printf( "Tr = %.3f - Tw = %.3f - Delta = %.4f | R=%d - W=%d - Delta = %d/%d (%.2f%%)\n", timeRead, timeWrite, deltaTime, U32(transportFromCentral.m_sampleIndexRead), U32(transportFromCentral.m_sampleIndexWrite), deltaRW, expectedDeltaRW, 100.0f * deltaRW / expectedDeltaRW );

			lastTime = now;
		}
	}
#endif

#if 0	// To debug microphone volume & auto-gain function
	{
		static U32	lastTime = millis();

		U32	now = millis();
		U32	elapsedTime_ms = now - lastTime;
		if ( elapsedTime_ms > 100 ) {

			U32	gainFactor = microphone.GetGainFactor();

			U32	averageVolume = microphone.m_audioPacketsCount > 0 ? microphone.m_sumVolume / microphone.m_audioPacketsCount : 0;
			microphone.m_sumVolume = 0;
			microphone.m_audioPacketsCount = 0;

			Serial.printf( "Volume = %d (avg %d) - Gain = %d - Silence #%d - Auto-Gain Avg. Volume = %d %s\n", microphone.m_volume, averageVolume, gainFactor, microphone.GetSilentSurroundCounter(), microphone.m_autoGainAverageVolume, transportToCentral.m_blockPackets ? " [PACKETS BLOCKED!!]" : "" );

			lastTime = now;
		}
	}
#endif

#if 0	// Show some packet stats
	#if defined(RECEIVE_AUDIO) || defined(TRANSMIT_AUDIO)
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
				#ifndef USE_DUMMY_MICROPHONE
					U32	receivedSamplesCount = microphone.m_sampleIndexWrite - lastReceivedSamplesCount;
				#else
					U32	receivedSamplesCount = 0;
				#endif
				U32	lastSentPacketID = transportToCentral.m_sentPacketsCount-1;
			#else
				U32	sentPackets = 0;
				U32	receivedSamplesCount = 0;
				U32	lastSentPacketID = -1;
			#endif

			// Debug ESP-Now packets status
			Serial.printf( "Rcv. %d, ID=0x%06X = %.1fHz - Lost = %2d (%.1f%%) | Sent %d, ID=0x%06x = %.1fHz | Mic Gain = %d (Silence %d)\n", receivedPackets, lastReceivedPacketID, TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * receivedPackets) / elapsedTime_ms, lostPackets, 100.0f * lostPackets / receivedPackets, sentPackets, lastSentPacketID, 2*TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * sentPackets) / elapsedTime_ms, microphone.GetGainFactor(), microphone.m_silentSurroundCounter );

			// Debug microphone input levels and sampling frequency
//			Serial.printf( "Rcv. %d samples from microphone [%d, %d] - Frequency = %.1f Hz | Mic Gain = %d\n", U32(microphone.m_sampleIndexWrite), microphone.m_sampleMin, microphone.m_sampleMax, (1000.0f * receivedSamplesCount) / (now - lastTime), microphone.GetGainFactor() );

			#ifdef RECEIVE_AUDIO
				lastReceivedPacketsCount = transportFromCentral.m_receivedPacketsCount;
				lastLostPacketsCount = transportFromCentral.m_lostPacketsCount;
			#endif
			#ifdef TRANSMIT_AUDIO
				lastSentPacketsCount = transportToCentral.m_sentPacketsCount;
				#ifndef USE_DUMMY_MICROPHONE
					lastReceivedSamplesCount = microphone.m_sampleIndexWrite;

					microphone.m_sampleMin = 0;
					microphone.m_sampleMax = 0;
				#endif
			#endif
			lastTime = now;
		}
	#endif
#endif
}

#endif	// defined(BUILD_PERIPHERAL)
