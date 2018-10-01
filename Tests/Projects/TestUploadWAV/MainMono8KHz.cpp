//////////////////////////////////////////////////////////////////////////
// The idea here is to have a 256 buffer where the reader (i.e. arduino) is 128 bytes
//	behind the writer (i.e. PC), the PC is going to write as fast as possible when
//	the arduino requires a new sample
//
// I decided to arbitrarily slice the sample as packets of 32 8-bits samples
// 
// We need some kind of protocol to enable:
//	1) The arduino to send a packet request at time t
//	2) the PC to send the pack at time t or initiate a new audio stream
//
// So basically, an audio packet must have a time stamp so it's written at
//	the proper place in the output buffer, we must be fault-tolerant and
//	can't assume all packets will be received!
//
//	• Let's decide a time unit is 16 samples (whether they're mono or stereo, 8- or 16-bits, it is of no importance)
//		At 8 KHz, that's 8000 samples per second, or 500 time units
//
//	• Let's assume we will send each audio packet along with a 3 bytes timestamp
//		That's 2^28 samples, at 8 KHz mono that means 33554 seconds or more than 9 hours of recording.
//
//	• Let's choose that each audio packet is 32 bytes long
//		With a 1 byte ID and 3 bytes timestamp as a header, that's 36 bytes to transfer
//		We need a 8KHz frequency, that's 8000/32 = 250 packets per second
//		Bandwidth is thus 250 * 36 * 8 = 70000 Bps
//
//////////////////////////////////////////////////////////////////////////
//
#if 1

#include "Pom/Pom.h"

// Pins to control the TLC7528 DAC
#define PIN_DACB	8			// !DACA/DACB
#define PIN_WR		9			// !WR

static const float	F = 8.0f;	// Channel restitution frequency (KHz)
static const U32	BR = 115200;// Baud Rate

//U32		writeTimeStamp = 128;	// Always ahead of read
U32		readTimeStamp = 0;		// Always behind write
U8		waveOut[256];			// Play buffer


//////////////////////////////////////////////////////////////////////////
// User timer1 for generation
//
void setup2() {
	cli();	// Clear interrupt flag

	// Set data pins to output
	for ( int i=0; i < 8; i++ )
		pinMode( i, OUTPUT );

	pinMode( PIN_DACB, OUTPUT );
	pinMode( PIN_WR, OUTPUT );

	memset( waveOut, 0, 256 );

	Timer1::Init( Timer1::Clk1, Timer1::CTC_OCR1A );	// Clear Timer and Compare Match => Will interrupt when counter == OCR1A
	Timer1::SetOutputCompareA_CTCFrequency( F );		// Interrupt at 8KHz (allows up to 4KHz reproductible frequency)
	Timer1::EnableInterrupts( true );

	//////////////////////////////////////////////////////////////////////////
	// Prepare serial interface
	UCSR0A = _BV(U2X0);	// u2x (double rate) mode

	// Assign the baud_setting, a.k.a. ubrr (USART Baud Rate Register)
	// As given by table 20-1, equation for computing the UBRR register value in asynchronouse double-speed mode is:
	//	UBRRn = Fosc / (8 * BaudRate) - 1
	//
	U16	baudSetting = F_CPU / 8 / BR - 1;
	UBRR0L = baudSetting;
	baudSetting >>= 8;
	UBRR0H = baudSetting;

	// Set the data bits, parity, and stop bits
	UCSR0C = SERIAL_8N1;	// Asynchronous + No parity + 8-bits + no stop bit (for TX only, TX is disabled anyway)
  
	UCSR0B = _BV(RXCIE0)	// Enable "Receive Complete" interrupt
		   | _BV(UDRIE0)	// Enable "Data Register Empty" interrupt
		   | _BV(RXEN0)		// Enable receiver
		   | _BV(TXEN0);	// Enable transmitter

	// Start main loop
	sei();
}

//////////////////////////////////////////////////////////////////////////
// Audio packet reception
//
#define AUDIO_PACKET_SIZE	(1 + 3 + 32)	// 1 byte = Packet ID
											// 3 bytes = timestamp
											// 32 bytes = audio content
U8	audioPacketIndex = 0;
U8	audioPacket[AUDIO_PACKET_SIZE];

U32	DEBUG_timeStamp = 0;
U32	DEBUG_TransmitDataCount = 0;

ISR( USART_RX_vect ) {

	// Read packet bytes
	audioPacket[audioPacketIndex++] = UDR0;
	if ( audioPacketIndex < AUDIO_PACKET_SIZE )
		return;	// Packet is not done yet...

	// Packet is complete!
	// Let's try and copy it to the proper place
	//
	audioPacketIndex = 0;	// Reset packet

	if ( audioPacket[0] != 0xAB )
		return;	// Invalid ID, ignore packet

	U32	timeStamp = (((audioPacket[1] << 8) | audioPacket[2]) << 8) | audioPacket[3];
	U32	sampleIndex = timeStamp << 5;	// A single packet contains 32 samples

	// Copy packet to wave out
	U8	writeIndex = 128 + sampleIndex;	// Always half a buffer ahead of read (hopefully)
	for ( U8 i=4; i < AUDIO_PACKET_SIZE; i++ ) {
//		waveOut[writeIndex++] = audioPacket[i];
//waveOut[writeIndex++] = i;
waveOut[writeIndex++] = sampleIndex + i;
	}

	// Send debug value
	if ( DEBUG_TransmitDataCount == 0 ) {
		DEBUG_timeStamp = timeStamp;
//DEBUG_timeStamp = 0x01234567;
		DEBUG_TransmitDataCount = 4;
		UCSR0B |= _BV(UDRIE0);	// Raise the interrupt to start transfer
	}
}

ISR( USART_UDRE_vect ) {
	if ( DEBUG_TransmitDataCount > 0 ) {
		// Transmit 32-bits debug value
		UDR0 = DEBUG_timeStamp;
		DEBUG_timeStamp >>= 8;
		DEBUG_TransmitDataCount--;
	}
}

//////////////////////////////////////////////////////////////////////////
// Audio replay
//
ISR( TIMER1_COMPA_vect ) {
//	digitalWrite( PIN_WR, HIGH );	// Hold write
	PORTB |= 2;

//	digitalWrite( PIN_DACB, LOW );
	U8	readIndex = readTimeStamp++;
	PORTB &= ~1;
	PORTD = waveOut[readIndex];

//	digitalWrite( PIN_WR, LOW );	// Resume write
	PORTB &= ~2;
}

//////////////////////////////////////////////////////////////////////////
//
void loop() {
	while ( true ) {
//		SerialPrintf( "Read %d bytes - 0x%x\n", readBytes, waveOut[0] );
	}
}

#endif	// MAIN_MONO_8BITS
