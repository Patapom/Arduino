////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Central Device using ESP-Now
// The Central receives voice streams from one or multiple Peripherals at 16KHz and sends hi-fi music at 44.1KHz to one or multiple Peripherals
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#if defined(BUILD_CENTRAL)

#include "./Common/Global.h"

#include "./Common/AudioBuffer.h"
#include "./Common/WAVFileSampler.h"
#include "./Common/ESPNow/TransportESPNow.h"

#include "./Common/WaveGenerator.h"
#include "./Common/AudioTransformers.h"

#include "Local.h"

#include "xtensa/core-macros.h"	// XTHAL_GET_CCOUNT()

char  str::ms_globalBuffer[256];
char* str::ms_globalPointer = str::ms_globalBuffer;

#define SEND_WAV_SAMPLE	// Define this to send the WAV sample, undefine to send back the received packets

#ifdef SEND_WAV_SAMPLE
WAVFileSampler	WAV;
AudioBuffer		WAVBuffer;	// We must insert a buffer between the WAV file and the ESP-Now uploader to avoid reading directly from the SPIFFS
#else
AudioBuffer		microphone2HiFi;
WaveGenerator	microphoneDummy;
//TransformMono2Stereo	transformerMono2Stereo;
TransformInterpolateMono2Stereo	transformerMono2Stereo;
#endif

TransportESPNow_Transmitter	transportToPeripherals;
TransportESPNow_Receiver	transportFromPeripherals;

//U32		timerCounter = 0;
bool	sendPacket = false;

void IRAM_ATTR	Timer0_ISR() {
//	timerCounter++;
	sendPacket = true;
}

void setup() {
	pinMode( PIN_LED_RED, OUTPUT );

	Serial.begin( 115200 );
	Serial.println();
	Serial.println( str( "Clock Frequency = %d.%03d MHz", U16( F_CPU / 1000000L ), U16( (F_CPU / 1000L) % 1000UL ) ) );

	ERROR( !SPIFFS.begin( false ), "An Error has occurred while mounting SPIFFS!" );

#ifdef SEND_WAV_SAMPLE
	// Open the WAV file & build the audio buffer
	ERROR( !WAV.Init( "/Alarm03.wav" ), "Failed to open the WAV file!" );
	ERROR( !WAVBuffer.Init( WAV, 1024, false ), "Failed to create the audio buffer between WAV file and ESP-Now" );
#endif

	// Initialize WiFi
  	WiFi.mode( WIFI_STA );
  	WiFi.disconnect();	// We're using ESP-Now
	Serial.println( str( "MAC Address is: %s", WiFi.macAddress().c_str() ) );

	// Initialize the ESP-Now transports
	transportToPeripherals.Init( ESP_NOW_WIFI_CHANNEL_CENTRAL_TO_PERIPHERAL );

	Serial.println( str( "Transport to Peripherals Initialized..." ) );

	transportFromPeripherals.SetSamplingRate( 16000 );
	transportFromPeripherals.Init( ESP_NOW_WIFI_CHANNEL_PERIPHERAL_TO_CENTRAL, 0x00, 2048 );	// We're the Central with special ID 0, we receive all the packets with bit mask 0!

	Serial.println( str( "Transport from Peripherals Initialized..." ) );

	// Initialize the timer that send high-fidelity audio packets at 44.1KHz to the Peripherals
	// We're targeting a 44100 Hz frequency but we can send 61 samples per packet, so we need a timer working a frequency of 722.9508 Hz
	// We achieve this using a base frequency of 224089 Hz (40MHz / 357) and counting up to 310, giving us a frequency of 722.869 Hz
	hw_timer_t*	timer0Config = timerBegin( 0, 357, true );	// 80MHz / 357 = 224089 Hz base frequency
//	timerAlarmWrite( timer0Config, 310, true );				// Count up to 310 => 722.869 Hz * 61 samples = 44095 Hz

// Half the packets sent => packets loss goes down to 1%! We need to compress our audio!
timerAlarmWrite( timer0Config, 620, true );				// Count up to 620 => 22047 Hz

	timerAttachInterrupt( timer0Config, &Timer0_ISR, true );
	timerAlarmEnable( timer0Config );

	Serial.println( str( "Timer enabled..." ) );

	// Build temporary audio buffer + transformer to convert a 16KHz mono sound into 22050Hz stereo
//	ERROR( !microphone2HiFi.Init( transportFromPeripherals, 2048 ), "Failed to initialize Microphone->HiFi Audio Buffer!" );

#ifndef SEND_WAV_SAMPLE
// Simulate a microphone input
microphoneDummy.SetChannelsCount( ISampleSource::MONO );
microphoneDummy.SetSamplingRate( 16000 );
microphoneDummy.SetWaveLeft( 1000 );
ERROR( !microphone2HiFi.Init( microphoneDummy, 2048 ), "Failed to initialize Microphone->HiFi Audio Buffer!" );

transformerMono2Stereo.Init( microphone2HiFi );	// Apply 16 KHz mono -> 22 KHz stereo transformer
transformerMono2Stereo.SetTargetSamplingRate( 22050 );
#endif

// Works!
//microphoneDummy.SetChannelsCount( ISampleSource::STEREO );
//microphoneDummy.SetSamplingRate( 22050 );
//microphoneDummy.SetWaveLeft( 1000 );
//microphoneDummy.SetWaveRight( 1000 );

}

U32		timerCounterSinceLastRead = 0;
U32		XTalCountSinceLastRead = 0;

void loop() {

	#ifdef SEND_WAV_SAMPLE	// Send WAV samples to the Peripherals
		// Should we send a packet?
		if ( sendPacket ) {
//			U32	packetID = timerCounter;	// Use the timer counter as packet ID
			U32	packetID = transportToPeripherals.m_sentPacketsCount;	// Use the transport's packet counter as packet ID, because the timer counter may have changed since it asked us to send the packet
//			transport.WriteSamples( WAVBuffer, 248 );
			transportToPeripherals.SendPacket( WAVBuffer, packetID );
			sendPacket = false;
		}

		// Update buffer with 256 samples from the WAV file as soon as there's enough room for them...
		WAVBuffer.UpdateBuffer( 256 );
	#else
		// Should we send a packet?
		if ( sendPacket ) {
//			U32	packetID = timerCounter;	// Use the timer counter as packet ID
			U32	packetID = transportToPeripherals.m_sentPacketsCount;	// Use the transport's packet counter as packet ID, because the timer counter may have changed since it asked us to send the packet
//			transportToPeripherals.SendPacket( transportFromPeripherals, packetID );
//			transportToPeripherals.SendPacket( microphone2HiFi, packetID );
			transportToPeripherals.SendPacket( transformerMono2Stereo, packetID );
			sendPacket = false;
		}
	#endif

//* Show some packet stats
static U32	lastReceivedPacketsCount = 0;
static U32	lastSentPacketsCount = 0;
static U32	lastLostPacketsCount = 0;
//static U32	lastReceivedSamplesCount = 0;
static U32	lastTime = millis();

U32	now = millis();
U32	elapsedTime_ms = now - lastTime;
if ( elapsedTime_ms > 1000 ) {
	U32	receivedPackets = transportFromPeripherals.m_receivedPacketsCount - lastReceivedPacketsCount;
	U32	lostPackets = transportFromPeripherals.m_lostPacketsCount - lastLostPacketsCount;
	U32	sentPackets = transportToPeripherals.m_sentPacketsCount - lastSentPacketsCount;
//	U32	receivedSamplesCount = input.m_sampleIndexWrite - lastReceivedSamplesCount;

	// Debug ESP-Now packets status
	Serial.printf( "Received %d, ID=0x%06X = %.1f Hz - Lost = %2d pcks (%.1f%%) | Sent %d, ID=0x%06X packets = %.1f Hz\n", receivedPackets, transportFromPeripherals.m_lastReceivedPacketID, 2*TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * receivedPackets) / elapsedTime_ms, lostPackets, 100.0f * lostPackets / receivedPackets, sentPackets, transportToPeripherals.m_sentPacketsCount-1, TransportESPNow_Base::SAMPLES_PER_PACKET * (1000.0f * sentPackets) / elapsedTime_ms );

	// Debug microphone input levels and sampling frequency
	//Serial.printf( "Received %d samples from microphone (Max = %d) - Frequency = %.1f Hz\n", input.m_sampleIndexWrite, input.m_sampleMax, (1000.0f * receivedSamplesCount) / (now - lastTime) );

//	input.m_sampleMax = 0;

	lastReceivedPacketsCount = transportFromPeripherals.m_receivedPacketsCount;
	lastLostPacketsCount = transportFromPeripherals.m_lostPacketsCount;
	lastSentPacketsCount = transportToPeripherals.m_sentPacketsCount;
//	lastReceivedSamplesCount = input.m_sampleIndexWrite;
	lastTime = now;
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