////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Central Device using ESP-Now
// The Central receives voice streams from one or multiple Peripherals at 16KHz and sends hi-fi music at 44.1KHz to one or multiple Peripherals
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#if defined(BUILD_CENTRAL)

//#define USE_SERIAL_FOR_DEBUG	// Define this to use serial for debugging, undefine to use it for actual high-speed data transfer with the PC
//#define	USE_CENTRAL_SERIAL	// Define this for actual high-speed data transfer with the PC, undefine to use it to use serial for debugging
#ifdef USE_CENTRAL_SERIAL
//#define NO_GLOBAL_SERIAL	// Use our Serial instead
#endif

#include "./Common/Global.h"

#include "./Common/AudioBuffer.h"
#include "./Common/ESPNow/TransportESPNow.h"
#include "./Common/AudioTools/WAVFileSampler.h"
#include "./Common/AudioTools/AudioTransformers.h"
#include "./Common/AudioTools/WaveGenerator.h"
#include "./Common/Serial/CentralSerial.h"

#include "Local.h"




// Discussions sur les CODECs et les retry values 
https://github.com/pschatzmann/arduino-audio-tools/discussions/393




char  str::ms_globalBuffer[256];
char* str::ms_globalPointer = str::ms_globalBuffer;

#ifndef USE_CENTRAL_SERIAL
	#define SEND_WAV_SAMPLE			// Define this to send the WAV sample, undefine to send back the received packets (i.e. microphone feedback with a twist!)
//	#define USE_DUMMY_MICROPHONE	// Define this to use a dummy microphone in place of the received signal
#endif

DefaultTime		mainTime;

CentralSerial	centralSerial( mainTime );
//HardwareSerial  Serial2( 2 );

TransportESPNow_Transmitter	transportToPeripherals( mainTime );
TransportESPNow_Receiver	transportFromPeripherals( mainTime );

#ifndef USE_CENTRAL_SERIAL
	#ifdef SEND_WAV_SAMPLE
		WAVFileSampler	WAV( mainTime );
	#else
		#ifndef USE_DUMMY_MICROPHONE	// Send back the received microphone input, after conversion from mono to stereo
			TransformMono2Stereo	mono2Stereo( transportFromPeripherals );
		#else							// Generate a waveform instead of microphone
			WaveGenerator			microphoneDummy;
			TransformMono2Stereo	mono2Stereo( microphoneDummy );
		#endif
	#endif
#endif

//S16	gs_sine[16384];

void setup() {
	pinMode( PIN_LED_RED, OUTPUT );

//	InitSine();

	Serial.begin( 115200 );
	Serial.println();
	Serial.println( str( "Clock Frequency = %d.%03d MHz", U16( F_CPU / 1000000L ), U16( (F_CPU / 1000L) % 1000UL ) ) );

	ERROR( !SPIFFS.begin( false ), "An Error has occurred while mounting SPIFFS!" );

	#ifdef USE_CENTRAL_SERIAL
		// We need at least a 1,411,200 bps full-duplex rate to transfer 44.1KHz stereo 16-bits samples
//		centralSerial.Init( CentralSerial::UART2, 2000000UL, GPIO_NUM_17, GPIO_NUM_16 );	// Doesn't work!
//		centralSerial.Init( CentralSerial::UART2, 1000000UL, GPIO_NUM_17, GPIO_NUM_16 );	// Lots of bytes lost! => Has to be a multiple of 115200, like shown below...
//		centralSerial.Init( CentralSerial::UART2, 2187500UL, GPIO_NUM_17, GPIO_NUM_16 );	// Maximum allowed on Windows... But doesn't work.
//		centralSerial.Init( CentralSerial::UART2, 1728000UL, GPIO_NUM_17, GPIO_NUM_16 );	// 15 x 115200 => Doesn't work...
//		centralSerial.Init( CentralSerial::UART2, 1382400UL, GPIO_NUM_17, GPIO_NUM_16 );	// 12 x 115200 => Doesn't work...

		// WORKS!
//		centralSerial.Init( CentralSerial::UART2, 1152000UL, GPIO_NUM_17, GPIO_NUM_16 );	// 10 x 115200 => Works nicely! But we lose so many packets @ 22kHz
		centralSerial.Init( CentralSerial::UART2, 8 * 115200UL, GPIO_NUM_17, GPIO_NUM_16 );	// 8 x 115200 => Enough to receive 16KHz stereo without losing too many packets

//		Serial2.begin( 115200 );
//		ERROR( true, "Serial configuré!! Error depuis le setup()!" );
	#endif

	#ifdef SEND_WAV_SAMPLE
		// Open the WAV file audio buffer
		ERROR( !WAV.Init( "/Alarm03.wav", 0.5f ), "Failed to open the WAV file!" );

		// Auto-update audio buffer in a task
		// We can't update in the main loop as it's too slow and will mess with packet sending => we won't be able to send as many packets as we'd like!
		WAV.StartAutoUpdateTask( 3 );
	#endif

	// ========================================================================================
	// Initialize WiFi
	TransportESPNow_Base::ConfigureWiFi( ESP_NOW_WIFI_CHANNEL );


	// ========================================================================================
	// Central -> Peripheral
	//
	transportToPeripherals.Init( CENTRAL_TO_PERIPHERAL_RATE );

	U8	receiverMaskID = 0xFF;	// Broadcast to every Peripheral by default...

	#ifdef SEND_WAV_SAMPLE	// Send WAV samples to the Peripherals
		transportToPeripherals.StartAutoSendTask( 4, WAV, receiverMaskID );
	#else	// Directly plug back what we received
		#ifdef USE_DUMMY_MICROPHONE
			microphoneDummy.SetChannelsCount( ISampleSource::MONO );
			microphoneDummy.SetWaveLeft( 1000.0f, 0.1f );
			microphoneDummy.SetWaveRight( 1000.0f, 0.1f );
//microphoneDummy.SetChannelsCount( ISampleSource::STEREO );
		#endif

		#ifdef USE_CENTRAL_SERIAL
			centralSerial.StartAutoReceiveTask( 2, transportToPeripherals );
		#else
			transportToPeripherals.StartAutoSendTask( 4, mono2Stereo, receiverMaskID );
//			transportToPeripherals.StartAutoSendTask( 4, microphoneDummy, receiverMaskID );
		#endif

	#endif

	Serial.println( str( "Transport to Peripherals Initialized..." ) );

	// ========================================================================================
	// Peripheral -> Central
	//
	transportFromPeripherals.Init( 0x00, PERIPHERAL_TO_CENTRAL_RATE, ISampleSource::MONO, 0.5f );	// We're the Central with special ID 0, we receive all the packets with bit mask 0!

	#ifdef USE_CENTRAL_SERIAL
		centralSerial.StartAutoSendTask( 4, transportFromPeripherals );
	#endif

	Serial.println( str( "Transport from Peripherals Initialized..." ) );

	// Start the show!
	mainTime.Start();
}

U32		timerCounterSinceLastRead = 0;
U32		XTalCountSinceLastRead = 0;

void loop() {

// DON'T UPDATE IN MAIN LOOP! It's too heavy and will prevent sending as many packets as we need!
//		// Update buffer with samples from the WAV file as soon as there's enough room for them...
//		WAV.Update();



//* Show some packet stats
static U32	lastReceivedPacketsCount = 0;
static U32	lastSentPacketsCount = 0;
static U32	lastLostPacketsCount = 0;
//static U32	lastReceivedSamplesCount = 0;
static U32	lastInvalidPacketsUSB = 0;
static U32	lastSearchBytesCount = 0;
static U32	lastUpdatesCount = 0;
static U32	lastTime = millis();

U32	now = millis();
U32	elapsedTime_ms = now - lastTime;
if ( elapsedTime_ms > 1000 ) {
	U32	receivedPackets = transportFromPeripherals.m_receivedPacketsCount - lastReceivedPacketsCount;
	U32	lostPackets = transportFromPeripherals.m_lostPacketsCount - lastLostPacketsCount;
	U32	sentPackets = transportToPeripherals.m_sentPacketsCount - lastSentPacketsCount;
	U32	invalidPacketsUSB = centralSerial.m_invalidPacketsCount - lastInvalidPacketsUSB;
	U32	searchedBytesCount = centralSerial.m_searchedBytesCount - lastSearchBytesCount;
//	U32	receivedSamplesCount = input.m_sampleIndexWrite - lastReceivedSamplesCount;
	#ifdef SEND_WAV_SAMPLE
		U32	updatesCount = WAV.m_updatesCount - lastUpdatesCount;
	#else
		U32	updatesCount = 0;
	#endif

	float	averageSearchedBytes = float( searchedBytesCount ) / invalidPacketsUSB;

	// Debug ESP-Now packets status
	Serial.printf( "Rcv. %d, ID=0x%06X = %.1fHz - Lost = %2d (%.1f%%) | Sent %d ID=0x%06X = %.1fHz - Lost USB %d - Avg. Searched Bytes %f / pckt - Upd# = %d\n", receivedPackets, transportFromPeripherals.m_lastReceivedPacketID, 2*TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * receivedPackets) / elapsedTime_ms, lostPackets, 100.0f * lostPackets / receivedPackets, sentPackets, transportToPeripherals.m_sentPacketsCount-1, TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * sentPackets) / elapsedTime_ms, invalidPacketsUSB, averageSearchedBytes, updatesCount );

	// Debug microphone input levels and sampling frequency
	//Serial.printf( "Received %d samples from microphone (Max = %d) - Frequency = %.1f Hz\n", input.m_sampleIndexWrite, input.m_sampleMax, (1000.0f * receivedSamplesCount) / (now - lastTime) );

//	input.m_sampleMax = 0;

	lastReceivedPacketsCount = transportFromPeripherals.m_receivedPacketsCount;
	lastLostPacketsCount = transportFromPeripherals.m_lostPacketsCount;
	lastSentPacketsCount = transportToPeripherals.m_sentPacketsCount;
	lastInvalidPacketsUSB = centralSerial.m_invalidPacketsCount;
	lastSearchBytesCount = centralSerial.m_searchedBytesCount;

	#ifdef SEND_WAV_SAMPLE
		lastUpdatesCount = WAV.m_updatesCount;
	#endif
//	lastReceivedSamplesCount = input.m_sampleIndexWrite;
	lastTime = now;

// Just a simple test
//	#ifdef USE_CENTRAL_SERIAL
//		centralSerial.Write();
////		Serial2.println( "Coucou!" );
//	#endif
}
//*/

/*	// Show timer frequency
	uint32_t XTalCount = XTHAL_GET_CCOUNT();
	float	elapsedTime = (XTalCount - XTalCountSinceLastRead) / 240000000.0;
	if ( elapsedTime >= 1.0f ) {
		Serial.println( str( "ElapsedTime = %f - Timer frequency = %f", elapsedTime, (timerCounter - timerCounterSinceLastRead) / elapsedTime ) );
		timerCounterSinceLastRead = timerCounter;
		XTalCountSinceLastRead = XTalCount;
	}
//*/
}

#endif
