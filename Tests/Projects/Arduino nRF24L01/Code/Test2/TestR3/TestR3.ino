////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transmitter Module
//  • Samples the mic at 32 KHz using a timer
//  • Stores the samples in a buffer and signals the nRF24L01 to send a packet whenever we collected 32 samples
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"
#include "nRF24L01.h"
#include <RF24.h>

static char* str::ms_globalBuffer = s_globalBuffer;

nRF24L01  radio( PIN_CE, PIN_CSN );

U8  sine[256];
U8  saw[256];

volatile U8   generalCounter = 0; // Use to read precomputed sine wave
volatile U8   samplesCounter = 0;
volatile bool packetReady = false;
volatile U8   samples[64];  // The tiny samples buffers
volatile U8*  samples0;     // This buffer is read from and sent as a radio packet
volatile U8*  samples1;     // This buffer is written to with samples collected from the mic

// Timer interrupt routine
U32 sentPacketsCount = 0;

void  setup() {
  pinMode( PIN_LED_RED, OUTPUT );
  pinMode( A0, INPUT );

float FREQ = 1.0; // 1 = 125 Hz (base frequency)
//float FREQ = 4.0; // 4 = 500 Hz
//float FREQ = 16.0; // 16 = 2000 Hz
//float FREQ = 64.0; // 64 = 8000 Hz
//float FREQ = 128.0; // 128 = 16000 Hz (Nyquist frequency)
for ( U32 i=0; i < 256; i++ ) {
  sine[i] = 127.5 + 127.5 * sin( 2*3.1415926 * FREQ * (0.5+i) / 256 );  // At 32 kHz, we need 256 samples to complete a cycle so the base output frequency is 32000 / 256 = 125 Hz
  saw[i] = i;
}

  // Initialize sample buffers pointers
  samples0 = &samples[0];
  samples1 = &samples[32];

  // Initiate serial communication
  Serial.begin( 19200 );
  while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

  Serial.println();
  Serial.println( str( "Clock Frequency = %d.%03d MHz", U16( F_CPU / 1000000L ), U16( (F_CPU / 1000L) % 1000UL ) ) );

  radio.Init( radio.PIPE0 );  // Only enable data pipe 0
  radio.SetAirDataRateAndPower( radio.DR_2MBPS, radio.PL_0dBm );
  U8  addr[] = { 0xE6, 0xE7, 0xE8, 0xE9, 0xEA };
  radio.SetTransmitterAddress( addr );
  radio.SetCRCMode( radio.CRC_1BYTE );  // Will be forced to "disabled" if auto-ACK is disabled...

//  radio.DumpRegisters();
//  Serial.println();

  // Start radio
  #ifdef USE_INTERRUPTS
    attachInterrupt( digitalPinToInterrupt( PIN_IRQ ), RadioIRQHandler, FALLING );
  #endif

  #ifdef USE_AUTO_ACK
    radio.PTX( true );  // Warning! Very slow!
  #else
    radio.PTX( false );
  #endif
  
  radio.Enable();

  Serial.println( "Radio enabled..." );

  // Enable ADC (cf. chapter 24 in the ATMEL doc)
  //
  // ADMUX Bits are:  REFS1 REFS0 ADLAR – MUX3 MUX2 MUX1 MUX0
  //
  ADMUX = _BV(REFS0)  // Enable the ADC PIN and set 5v Analog Reference
        | _BV(ADLAR)  // Left-adjust result so only high byte needs to be read in ADCH (i.e. 8-bits conversion)
        | 0;          // Use A0 as source

  // ADCSRB Bits are: – ACME – – – ADTS2 ADTS1 ADTS0
  //  • ACME is probably "mux extension" used to support additional inputs
  //  • ADTS set the possible trigger sources:
  //      000 Free Running mode
  //      001 Analog Comparator
  //      010 External Interrupt Request 0
  //      011 Timer/Counter0 Compare Match A
  //      100 Timer/Counter0 Overflow
  //      101 Timer/Counter1 Compare Match B
  //      110 Timer/Counter1 Overflow
  //      111 Timer/Counter1 Capture Event
  //
  ADCSRB = 0*_BV(ADTS2) | 0*_BV(ADTS1) | 0*_BV(ADTS0);  // Free-running mode (i.e. as fast as possible)

  // ADCSRA Bits are: ADEN   ADSC   ADATE  ADIF    ADIE    ADPS2 ADPS1 ADPS0
  //                  enable start  auto-  inter.  inter.  prescaler bits
  //                         conv.  trig.  flag    enable  001=2 -> 111=128
  //
  // A single conversion uses 13 clock cycles (pp. 240), so the various prescaler values would yield:
  //  prescale = 128 => F = 16 MHz / 128 / 13 = 9615 Hz
  //  prescale = 64  => F = 19,230 Hz
  //  prescale = 32  => F = 38,461 Hz
  //  prescale = 16  => F = 76,923 Hz
  //  prescale = 8   => F = 153,846 Hz
  //  prescale = 4   => F = 307,692 Hz
  //  prescale = 2   => F = 615,384 Hz
  //
  ADCSRA = _BV(ADEN)  // ADC Enable
         | _BV(ADSC)  // Start conversion
         | _BV(ADATE) // Auto-trigger enable
         | 0x05;      // Prescaler set to 32 for 38461 Hz sampling rate


//*
  // Start a 32 KHz timer that will sample the mic (code from https://www.instructables.com/Arduino-Audio-Output/)
  cli();    // Disable interrupts
//  TCCR2A = 0; // Output Compare pins => COM0A:2 = 00 (OC0A disconnected) | COM0B:2 = 00 (OC0B disconnected) | 00 | WGM0 = 00 (Normal)
//  TCCR2B = 0; // FOC0A:2 = 00 (Force Output Compare A) | FOC0B:2 = 00 | -- | WGM2:1 = 0 | CS0:3 = 000 (No clock source, timer/counter stopped)

  // Set compare match register for 16khz increments
  TCCR2A = 0b00000010;  // Turn on Clear Timer on Compare Match (CTC) mode that will signal an interrupt when the counter reaches the value sets in OCR0A
  TCCR2B = 0b00000010;  // Set clock/8 prescaler (001=1, 010=8, 011=64, 100=256, 101=1024)
  TCNT2  = 0;           // Timer/Counter Register = 0
  OCR2A = U32(F_CPU) / (SAMPLING_RATE * 8) - 1;  // Compare match value = (16,000,000) / (frequency * prescaler) - 1 (must be <256 for timer 0 and timer 2, timer 1 is 16 bits)
  TIMSK2 = 0b00000010;  // Enable timer compare interrupt

  sei();  // Enable interrupts

  Serial.println( str( "Timer 2 enabled (OCR2A = 0x%02X => Freq = %u Hz)...", OCR2A, U32( F_CPU ) / U32( (OCR2A + 1) * 8UL ) ) );

//*/
}

ISR(TIMER2_COMPA_vect) {
  // Unfortunately, we can't sample the ADC at high frequency using analogRead() (cf. https://forum.arduino.cc/t/timer1-interrupt-debuging/167256/3)
//  samples1[samplesCounter] = analogRead( A0 ); // Sample the MAX9814 (Microphone amplifier)

  // Instead, let's read the ADC register and a continuous, free-running sampling configuration of the ADC...
  samples1[samplesCounter] = ADCH;  // Sample the MAX9814 (Microphone amplifier)

//samples1[samplesCounter] = sine[generalCounter];
//samples1[samplesCounter] = saw[generalCounter];
//samples1[samplesCounter] = (sentPacketsCount & 0x400UL) ? saw[generalCounter] : sine[generalCounter];  // Alternate between sine/saw signals
//samples1[samplesCounter] = (sentPacketsCount & 0x400UL) ? saw[generalCounter] : ~saw[generalCounter];  // Alternate between upward/downward saw

  generalCounter++;
  samplesCounter++;
  samplesCounter &= 0x1F;

  // Raise the "packetReady" flag as soon as we collected 32 samples
  // This should occur 32000 / 32 = 1000 times per second...
//  packetReady |= samplesCounter == 0;
  if ( samplesCounter == 0 ) {
    // Swap sample buffers
    volatile U8* temp = samples0;
    samples0 = samples1;
    samples1 = temp;
    packetReady = true;
  }

// DON'T try to send packets within the interrupt! It looks like the SPI isn't working correctly...
/*
  if ( packetReady ) {
    packetReady = false;
    SendPacket( samples );
  }
//*/

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

  SendPacket( samples0 );

  // Clear packet ready flag...
  packetReady = false;
//*/
}

void  SendPacket( U8* _samples ) {
  // Write the packet into the TX FIFO
  if ( !radio.WritePayload( _samples ) )
    return; // FIFO is full, can't send...

  // Transmission should start ASAP, since the radio chip is always enabled...

  #ifndef USE_INTERRUPTS
    // Wait until the TX_DS flag is set, then clear it...
//delayMicroseconds( 250 );
    while ( radio.PacketSent() != radio.PSS_SENT );
    radio.ClearInterruptFlags( _BV(TX_DS) );
  #endif

#if 1
  // Shine the LED every 128 packets, so a frequency of about 32000 / (128 * 32) ~= 7.8 Hz
  sentPacketsCount++;
  digitalWrite( PIN_LED_RED, sentPacketsCount & 0x80 );
#endif
}

#ifdef USE_INTERRUPTS
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
#endif
