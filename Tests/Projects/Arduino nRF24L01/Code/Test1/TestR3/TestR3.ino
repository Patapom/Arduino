////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transmitter Module
//  • Samples the mic at 16KHz using a timer
//  • Stores the samples in a buffer and signals the nRF24L01 to send the samples
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"
#include "nRF24L01.h"
#include <RF24.h>

static char* str::ms_globalBuffer = s_globalBuffer;

nRF24L01  radio( PIN_CE, PIN_CSN );

U8  sine[256];
U8  saw[256];

void  setup() {
  pinMode( PIN_LED_RED, OUTPUT );
  pinMode( A0, INPUT );

for ( U32 i=0; i < 256; i++ ) {
  sine[i] = 127.5 + 127.5 * sin( 2*3.1415926 * i / 256 );
  saw[i] = i;
}

  // Initiate serial communication
  Serial.begin( 19200 );
  while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

  Serial.println();
  Serial.println( str( "Clock Frequency = %d.%03d MHz", U16( F_CPU / 1000000L ), U16( (F_CPU / 1000L) % 1000UL ) ) );

  radio.Init( radio.PIPE0 );  // Only enable data pipe 0
  radio.SetAirDataRateAndPower( radio.DR_2MBPS, radio.PL_0dBm );
  U8  addr[] = { 0xE6, 0xE7, 0xE8, 0xE9, 0xEA };
  radio.SetTransmitterAddress( addr );

//  radio.DumpRegisters();
//  Serial.println();

  // Start radio
  radio.PTX();
  attachInterrupt( digitalPinToInterrupt( PIN_IRQ ), RadioIRQHandler, LOW );
  radio.Enable();

  Serial.println( "Radio enabled..." );

//*
  // Start a 32KHz timer that will sample the mic (code from https://www.instructables.com/Arduino-Audio-Output/)
  cli();    // Disable interrupts
//  TCCR2A = 0; // Output Compare pins => COM0A:2 = 00 (OC0A disconnected) | COM0B:2 = 00 (OC0B disconnected) | 00 | WGM0 = 00 (Normal)
//  TCCR2B = 0; // FOC0A:2 = 00 (Force Output Compare A) | FOC0B:2 = 00 | -- | WGM2:1 = 0 | CS0:3 = 000 (No clock source, timer/counter stopped)

  // Set compare match register for 16khz increments
  TCCR2A = 0b00000010;  // Turn on Clear Timer on Compare Match (CTC) mode that will signal an interrupt when the counter reaches the value sets in OCR0A
  TCCR2B = 0b00000010;  // Set clock/8 prescaler (001=1, 010=8, 011=64, 100=256, 101=1024)
  TCNT2  = 0;           // Timer/Counter Register = 0
//  OCR2A = 124;          // Compare match value = (16,000,000) / (frequency * prescaler) - 1 (must be <256 for timer 0 and timer 2, timer 1 is 16 bits)
  OCR2A = U32(F_CPU) / (32000L * 8) - 1;  // Compare match value = (16,000,000) / (frequency * prescaler) - 1 (must be <256 for timer 0 and timer 2, timer 1 is 16 bits)
//  OCR2A = 62;          // Compare match value = (16,000,000) / (frequency * prescaler) - 1 (must be <256 for timer 0 and timer 2, timer 1 is 16 bits)
  TIMSK2 = 0b00000010;  // Enable timer compare interrupt

  sei();  // Enable interrupts

  Serial.println( str( "Timer 2 enabled (OCR2A = 0x%02X => Freq = %u Hz)...", OCR2A, U32( F_CPU ) / U32( (OCR2A + 1) * 8UL ) ) );

//*/
}

volatile U8   generalCounter = 0; // Use to read precomputed sine wave
volatile U8   samplesCounter = 0;
volatile bool packetReady = false;
volatile U8   samples[32];  // The tiny samples buffer  

// Timer interrupt routine
U32 sentPacketsCount = 0;

ISR(TIMER2_COMPA_vect) {
//  samples[samplesCounter] = analogRead( A0 ); // Sample the MAX9814 (Microphone amplifier)
//samples[samplesCounter] = sine[generalCounter];
//samples[samplesCounter] = saw[generalCounter];
samples[samplesCounter] = (sentPacketsCount & 0x400UL) ? saw[generalCounter] : sine[generalCounter];    // Alternate between sine/saw signals
//samples[samplesCounter] = (sentPacketsCount & 0x400UL) ? saw[generalCounter] : ~saw[generalCounter];  // Alternate between upward/downward saw

  generalCounter++;
  samplesCounter++;
  samplesCounter &= 0x1F;

  // Raise the "packetReady" flag as soon as we collected 32 samples
  // This should occur 16000 / 32 = 500 times per second...
  packetReady |= samplesCounter == 0;

// DON'T try to send packets within the interrupt! It looks like the SPI isn't working correctly...
//  if ( samplesCounter == 0 )
//    SendPacket( samples );

#if 0
// Shine the LED every 128 packets, so a frequency of about 500 packets/second / 128 packets ~= 3.9Hz
if ( samplesCounter == 0 ) {
  sentPacketsCount++;
  digitalWrite( PIN_LED_RED, sentPacketsCount & 0x80 );
}
#endif
}

// The main loop only watches for sending packets out to the receiver...
void  loop() {
//*
  if ( !packetReady )
    return; // Nothing to transmit...

  SendPacket( samples );

  // Clear packet ready flag...
  packetReady = false;
//*/
}

void  SendPacket( U8* _samples ) {

  // Write the packet into the TX FIFO
  radio.Select();
  
  U8  status = SPI.transfer( W_TX_PAYLOAD );

  U8  payloadLength = 32;
  while ( payloadLength-- ) {
    SPI.transfer( *_samples++ );
  }

  radio.Deselect();

  // Transmission should start ASAP, since the radio chip is always enabled...

#if 1
// Shine the LED every 128 packets, so a frequency of about 500 packets/second / 128 packets ~= 3.9Hz
sentPacketsCount++;
digitalWrite( PIN_LED_RED, sentPacketsCount & 0x80 );
#endif
}

// Expected IRQ's are:
//  • TX_DS on packet successfully sent
//  • MAX_RT if packet failed after too many retransmits
//    => In which case we must flush the TX FIFO...
//
// In any case, we must clear the IRQ flags by writing to the NRF_STATUS register...
//
void  RadioIRQHandler() {
  // Clear interrupt flags
  radio.ClearInterruptFlags();	// This also stores the NRF_STATUS value into radio.m_status before writing into it!

  if ( radio.m_status & _BV(MAX_RT) ) {
    // Packet failed to send after too many retransmits...
    // We must flush the TX FIFO ourselves...
    radio.FlushTX();

//digitalWrite( PIN_LED_RED, true );  // Error...
  } else {
//digitalWrite( PIN_LED_RED, false );
  }
}

