////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sound Transmission via Bluetooth to the EMEET M0 Plus
// We make the ESP32 an audio source and attempt to connect to the EMEET to output some audio...
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#if defined(BUILD_RECEIVER)

#include "./Common/Global.h"
#include "./Common/AudioBuffer.h"
#include "./Common/I2SOutput/I2SOutput.h"
#include "./Common/I2SOutput/WAVFileSampler.h"
#include "./Common/ESPNow/TransportESPNow.h"

#include "Local.h"

//#define	USE_WAV	// Define this to use the WAV file as I2S sample source, undefine to use audio packets received via ESP-Now

char  str::ms_globalBuffer[256];
char* str::ms_globalPointer = str::ms_globalBuffer;


//////////////////////////////////////////////////////////////
#define PIN_I2S_DOUT  22  // Data out
#define PIN_I2S_BCLK  26  // Bits clock
#define PIN_I2S_LRC   25  // Left/Right Select

I2SOutput	output;

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

TransportESPNow_Receiver	transport;

ISampleSource&	InitSampleSource() {
	// Initialize WiFi
  	WiFi.mode( WIFI_STA );
  	WiFi.disconnect();	// We're using ESP-Now
	Serial.println( str( "MAC Address is: %s", WiFi.macAddress().c_str() ) );

	// Initialize the audio buffer
	U8	header[] = { 0x12, 0x34 };
	transport.SetHeader( sizeof(header), header );
	transport.SetSamplingRate( 44100 );
	transport.Init( ESP_NOW_WIFI_CHANNEL, 0x01, 2048 );	// Use ID 1

	return transport;
}

#endif

void  InitI2S() {

	ISampleSource&	sampleSource = InitSampleSource();

	i2s_pin_config_t  i2sPins = {
		.bck_io_num = PIN_I2S_BCLK,
		.ws_io_num = PIN_I2S_LRC,
		.data_out_num = PIN_I2S_DOUT,
		.data_in_num = -1
	};

	output.Start( I2S_NUM_0, i2sPins, sampleSource );

	output.SetVolume( 0.125f );

	Serial.println( "I2S Initialized..." );
}

void setup() {
	Serial.begin( 115200 );
	Serial.println( "Serial ready" );

	if ( !SPIFFS.begin( true ) ) {
		Serial.println( "An Error has occurred while mounting SPIFFS!" );
		return;
	}

	InitI2S();
}

void loop() {
	#ifdef UPDATE_IN_MAIN_LOOP
		WAVBuffer.UpdateBuffer( I2SOutput::NUM_SAMPLES_TO_SEND );	// Update with samples ASAP
	#else
	  	delay( 1000 );  // to prevent watchdog in release > 1.0.6

//* Show some packet stats
static U32	lastReceivedPacketsCount = 0;
static U32	lastLostPacketsCount = 0;
static U32	lastTime = millis();
U32	receivedPackets = transport.m_receivedPacketsCount - lastReceivedPacketsCount;
U32	lostPackets = transport.m_lostPacketsCount - lastLostPacketsCount;
lastReceivedPacketsCount = transport.m_receivedPacketsCount;
lastLostPacketsCount = transport.m_lostPacketsCount;

U32	now = millis();
Serial.printf( "Received %d packets = %.1f Hz - Lost packets = %d pcks (%.1f%%)\n", receivedPackets, 61 * (1000.0f * receivedPackets) / (now - lastTime), lostPackets, 100.0f * lostPackets / receivedPackets );
lastTime = now;
//*/

	#endif
}

#endif