////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Music Emitter through ESP-Now
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#if defined(BUILD_EMITTER)

#include "./Common/Global.h"

#include "./Common/AudioBuffer.h"
#include "./Common/I2SOutput/WAVFileSampler.h"
#include "./Common/ESPNow/TransportESPNow.h"

#include "Local.h"

#include "xtensa/core-macros.h"	// XTHAL_GET_CCOUNT()

#define BURST_TIMER

char  str::ms_globalBuffer[256];
char* str::ms_globalPointer = str::ms_globalBuffer;

WAVFileSampler	WAV;
AudioBuffer		WAVBuffer;	// We must insert a buffer between the WAV file and the ESP-Now uploader to avoid reading directly from the SPIFFS

TransportESPNow_Transmitter	transport;

void IRAM_ATTR	Timer0_ISR();

void setup() {
	pinMode( PIN_LED_RED, OUTPUT );

	Serial.begin( 115200 );
	Serial.println();
	Serial.println( str( "Clock Frequency = %d.%03d MHz", U16( F_CPU / 1000000L ), U16( (F_CPU / 1000L) % 1000UL ) ) );

	ERROR( !SPIFFS.begin( false ), "An Error has occurred while mounting SPIFFS!" );

	// Open the WAV file & build the audio buffer
	ERROR( !WAV.Init( "/Alarm03.wav" ), "Failed to open the WAV file!" );
	ERROR( !WAVBuffer.Init( WAV, 1024, false ), "Failed to create the audio buffer between WAV file and ESP-Now" );

	// Initialize WiFi
  	WiFi.mode( WIFI_STA );
  	WiFi.disconnect();	// We're using ESP-Now
	Serial.println( str( "MAC Address is: %s", WiFi.macAddress().c_str() ) );

	// Initialize the transport
	U8	header[] = { 0x12, 0x34 };
	transport.SetHeader( sizeof(header), header );
	transport.Init( ESP_NOW_WIFI_CHANNEL );

	#ifdef BURST_TIMER
		// We're targeting a 44100 Hz frequency but we can send 61 samples per packet, so we need a timer working a frequency of 722.9508 Hz
		// We achieve this using a base frequency of 224089 Hz (40MHz / 357) and counting up to 310, giving us a frequency of 722.869 Hz
		hw_timer_t*	timer0Config = timerBegin( 0, 357, true );	// 80MHz / 357 = 224089 Hz base frequency
//		timerAlarmWrite( timer0Config, 310, true );				// Count up to 310 => 722.869 Hz * 61 samples = 44095 Hz

timerAlarmWrite( timer0Config, 620, true );				// Count up to 620 => 22050 Hz (less packets lost!)

		timerAttachInterrupt( timer0Config, &Timer0_ISR, true );
		timerAlarmEnable( timer0Config );
	#else
		// Start a 44.1 kHz timer
		#if 1
			hw_timer_t*	timer0Config = timerBegin( 0, 2, true );	// 80MHz / 2 = 40MHz base frequency
			timerAttachInterrupt( timer0Config, &Timer0_ISR, true );
			timerAlarmWrite( timer0Config, 905, true );				// Count up to 907 => 44.101 kHz => After experimentation, turns out the value of 905 is achieving better frequency matching!
			timerAlarmEnable( timer0Config );
		#else	// Easy version!! But I don't have this version of the Espressif API???
			hw_timer_t*	timer0Config = timerBegin( 44100 ); // 44.1kHz
			timerAttachInterrupt( timer0Config, &Timer0_ISR );
			timerAlarm( timer0Config, 1, true, 0 ); // Signal every time counter reaches 1, i.e. use the full frequency of the timer...
			timerRestart( timer0Config );
		#endif
	#endif

	Serial.println( str( "Timer enabled..." ) );
}

bool	prout = true;
U32		timerCounter = 0;
U32		timerCounterSinceLastRead = 0;
U32		XTalCountSinceLastRead = 0;

bool	sendPacket = false;

void loop() {

//	digitalWrite( PIN_LED_RED, prout );
//	prout = !prout;
//	delay( 1000 );

	// Should we send a packet?
	if ( sendPacket ) {
//		transport.WriteSamples( WAVBuffer, 248 );
		transport.SendPacket( WAVBuffer, timerCounter );	// Use the timer counter as packet ID
		sendPacket = false;
	}

	// Update buffer with 256 samples from the WAV file as soon as there's enough room for them...
	WAVBuffer.UpdateBuffer( 256 );

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

// --------------------------------------------------------
// Timer IRQ
//
#ifdef BURST_TIMER
void IRAM_ATTR	Timer0_ISR() {
	timerCounter++;
////	WAVBuffer.GetSamples( tempSamples, 248 );
//	transport.WriteSamples( WAVBuffer, 248 );	// Kinda "works" too, but eventually reboots after a few loops of the sample... It's much better not to do any heavy lifting inside an IRQ!
	sendPacket = true;
}
#else
void IRAM_ATTR	Timer0_ISR() {
	timerCounter++;
	Sample	sample;
	
// Of course, SPIFFS is not supported in an IRQ handler!! :D
// We'll need to read from a temp buffer that will be loaded in the main loop:
//	• The timer indicates which sample it's requesting
//	• The main loop will read a large amount of samples in advance in some kind of double buffer
//
//	WAV.GetSamples( &sample, 1 );
	WAVBuffer.GetSamples( &sample, 1 );

////	sample.left = S16( 32767.5 + 32767.5 * sin( 6.28f * timerCounter / 88.2 ) );	// 500 Hz signal
////	sample.left = S16( 32767.5 + 32767.5 * sin( 6.28f * timerCounter / 44.1 ) );	// 1000 Hz signal
//	sample.left = S16( 32767.5 + 32767.5 * sin( 6.28f * timerCounter / 22.05 ) );	// 2000 Hz signal
//	sample.right = sample.left;

	transport.WriteSample( sample );
}

#endif
#endif
