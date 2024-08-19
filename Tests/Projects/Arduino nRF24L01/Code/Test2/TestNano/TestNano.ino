////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Receiver Module
//  • Uses IRQ pin to monitor received packets
//  • Uses a 16KHz timer to replay the samples received from the radio packets
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"
#include "nRF24L01.h"
#include "MCP4725.h"

static char* str::ms_globalBuffer = s_globalBuffer;

nRF24L01  radio( PIN_CE, PIN_CSN );
MCP4725   DAC( 0x60 );  // 0x60 to 0x67. My test device has the address 0x60 | (A2 << 2) | (A1 << 1) | A0 where A2 = A1 = A0 = 0 (its A0 pin connected to GND so A0 = 0)

// Check increasing the wire speed with TWBR = 12; // 400 khz  (from: https://learn.adafruit.com/mcp4725-12-bit-dac-tutorial/using-with-arduino) 

// Le MCP4725 accepte jusqu'à 3.4Mbps !
//  => Arduino va jusqu'à combien ????
//    => Clairement pas assez!

U16 currentSample = 0;

volatile U32  sampleIndex = 128UL << 8; // The current sample index, with 8 bits of fixed decimal precision (initialized 128 samples ahead of packets storage offset)
volatile U32  sampleInc = 0x100UL;      // The sample increment, with 8 bits of fixed decimal precision
U8            samples[256];             // The tiny circular buffer of samples

U32 receivedPacketsCount = 0;

U8 globalPacketOffset = 0;  // This will automatically cycle around our 256 bytes buffer...

void setup() {
  pinMode( PIN_LED_RED, OUTPUT );
  pinMode( PIN_IRQ, INPUT_PULLUP );
  pinMode( A0, INPUT );

  memset( samples, 0x80, 256 ); // Clear to 128 so the DAC sends half of Vcc if samples are not initialized...

  // Initiate serial communication
  Serial.begin( 19200 );
  while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

  Serial.println();
  Serial.println( str( "Clock Frequency = %d.%03d MHz", U16( F_CPU / 1000000L ), U16( (F_CPU / 1000L) % 1000UL ) ) );

  radio.Init( radio.PIPE0 );  // Only enable data pipe 0
  radio.SetAirDataRateAndPower( radio.DR_2MBPS, radio.PL_0dBm );
  U8  addr[] = { 0xE6, 0xE7, 0xE8, 0xE9, 0xEA };  // Must be the same as transmitter so transmitter packets are forwarded to data pipe 0
  radio.SetReceiverSingleAddress( addr );
  radio.SetCRCMode( radio.CRC_1BYTE );  // Will be forced to "disabled" if auto-ACK is disabled...

//  radio.DumpRegisters();
//  Serial.println();

  // Enable PRX
  #ifdef USE_INTERRUPTS
    attachInterrupt( digitalPinToInterrupt( PIN_IRQ ), RadioIRQHandler, FALLING );
  #endif

  #ifdef USE_AUTO_ACK
    radio.PRX( true );  // Warning! Very slow!
  #else
    radio.PRX( false ); // We will monitor any incoming packets continuously...
  #endif

  radio.Enable();

  Serial.println( "Radio enabled..." );

  // Enable DAC on I2C
  Wire.begin();
  Wire.setClock( 400000L );  // Increase I2C speed to 400kHz
//  Wire.setClock( 3400000L );  // Increase I2C speed to 3.4Mbps => Can't reach that, unfortunately!

TWBR = 1; // Lowest delay = fastest transmit rate

  DAC.begin();
  Serial.println( str( "DAC is %s", DAC.isConnected() ? "CONNECTED" : "**NOT** Connected" ) );

//*
  // Start a timer that will send samples to the DAC for replay
  cli();    // Disable interrupts

  TCCR2A = 0b00000010;  // Turn on Clear Timer on Compare Match (CTC) mode that will signal an interrupt when the counter reaches the value sets in OCR0A
  TCCR2B = 0b00000010;  // Set CS2 to clock/8 prescaler
  TCNT2  = 0;           // Timer/Counter Register = 0
  OCR2A = U32(F_CPU) / (SAMPLING_RATE * 8) - 1;  // Compare match value = (16,000,000) / (frequency * prescaler) - 1 (must be <256 for timer 0 and timer 2, timer 1 is 16 bits)
  TIMSK2 = 0b00000010;  // Enable timer compare interrupt

  sei();  // Enable interrupts

  Serial.println( str( "Timer 2 enabled (OCR2A = 0x%02X => Freq = %u Hz)...", OCR2A, U32( F_CPU ) / U32( (OCR2A + 1) * 8UL ) ) );
//*/
}

// Timer interrupt routine
// Ideally we'd like to send the sample to the DAC now but the I2C protocol is not functional during an interrupt request!
// Instead, we simply copy the sample value to be played by the main loop...
ISR(TIMER2_COMPA_vect) {
//  U8  sampleIndex8Bits = ((U8*) &sampleIndex)[2];
  U8  sampleIndex8Bits = U8( sampleIndex >> 8 );  // Probably cheaper
  currentSample = samples[sampleIndex8Bits] << 4; // DAC expects 12-bits samples, we only have 8-bits ones...

  sampleIndex += sampleInc;

// Check why we can't use I2C during an IRQ????
//  DAC.setValue( sample << 4 );

//*
  // Check if a packet is available
  U8  packetReceivedPipeIndex = radio.PacketReceived();
  if ( packetReceivedPipeIndex >= 6 )
    return;

  radio.ClearInterruptFlags( _BV(RX_DR) );

  // Read payload 128 bytes ahead of sampling pointer, aligned with 32-bytes packets
//  U8  packetOffset = (sampleIndex8Bits & 0xE0) + 0x80;

  // Just read along...
  U8  packetOffset = globalPacketOffset;
  globalPacketOffset += 32U;

//  sei();  // Enable interrupts again so we don't miss any sample
  radio.ReadPayload( samples + packetOffset );

  receivedPacketsCount++;
  digitalWrite( PIN_LED_RED, receivedPacketsCount & 0x80 );
//*/  
}

char* debugText = NULL;
void loop() {
  DAC.setValue( currentSample );

  // Notify every 128 packets (i.e. 128 * 32 samples at 32 KHz = every 128ms)
//  if ( (receivedPacketsCount & 0x7F) == 0 ) {
//    Serial.println( str( "Received %u packets...", receivedPacketsCount ) );
//  }


if ( debugText != NULL ) {
  Serial.println( debugText );
  debugText = NULL;
}

// Simple DAC tests
//  DAC.setValue( runCounter & 0x01FFU );
//  DAC.setValue( 2048 + 2047 * sin( 3.1415926 * runCounter / 1 ) );
//  DAC.setValue( ((runCounter >> 10) & 1) * 4095 );
//  DAC.setValue( values[runCounter & VALUES_MASK] );

//Serial.println( values[runCounter & VALUES_MASK] );

//Serial.println( str( "sampleIndex = 0x%08lX, sampleInc = 0x%08lX", sampleIndex, sampleInc ) );

  #ifndef USE_INTERRUPTS
    // For when we don't use the IRQ: constantly poll for a packet...
//    RadioIRQHandler();
//    FastPacketReader();
  #endif
}

//U32 lastCyclePacketOffset = 256UL << 8; // Starting at cycle 1
U32 firstPacketOffset = ~0UL;
U32 firstPacketSampleIndex = 0;

#ifdef USE_INTERRUPTS

void  RadioIRQHandler() {

//digitalWrite( PIN_LED_RED, 1 );
//return;

#if 0 // I saw this code fail as if the device hanged: my take is that maybe we receive a new interrupt while we're reading the payload, and so the code never escapes the interrupt routine...
      // I think we should only clear the interrupt flag once the payloads have indeed been read!

  // Apparently, pipe index is unreliable during a FALLING IRQ transition...
  // We should clear the interrupt flag first...
  radio.ClearInterruptFlags( _BV(RX_DR) );

    U8  status = radio.Status();
    U8  pipeIndex = (status & 0x0F) >> 1;
    if ( pipeIndex > 5 ) {
      // Inconsistent state where we received a packet but can't tell in which pipe!
//    digitalWrite( PIN_LED_RED, 1 );
      radio.FlushRX();
      return;
    }
#else
  U8  pipeIndex = radio.PacketReceived();
  if ( pipeIndex == 0xFF ) {
    // Some error occurred! We got notified of an IRQ but flag RX_DR is not set!
//    digitalWrite( PIN_LED_RED, 1 );
    radio.ClearInterruptFlags();
    return;
  } else if ( pipeIndex == 0xFE ) {
    // Inconsistent state where we received a packet but can't tell in which pipe!
//    digitalWrite( PIN_LED_RED, 1 );
    radio.ClearInterruptFlags();
    radio.FlushRX();
    return;
  }
#endif

  // We use the current sample index to determine where we should deposit the next packet
  //  • We know the datarate should be 16KHz so it's better to rely on the timer, as inconsistent it may be between 2 devices
  //  • We also use the fact that we know when we did a "full cycle" (i.e. when we receive enough packets to fill the entire buffer): it means the transmitter completed its own 256 samples cycle
  //    => We can use this information to devise and adjust the replay rate of the receiver so it matches the transmitter's
  //
  U32 packetOffset = (sampleIndex + (128UL << 8)); // We add an offset of 128 to target packets well ahead of the current position...
      packetOffset &= ~0x1FFFUL;                   // Make sure we have a stride of 32 bytes (i.e. integral packets)

  if ( firstPacketOffset == ~0UL ) {
    firstPacketOffset = packetOffset;
    firstPacketSampleIndex = sampleIndex;
  }

//debugText = str( "sampleIndex = 0x%08lX", sampleIndex );

  U8  tempPacketLength;
  U32 FIFOCount = 0;
  while ( !radio.IsRXFIFOEmpty() ) {
    FIFOCount++;
    // Read payload at computed offset
    U8  packetOffset8Bits = U8( packetOffset >> 8 );
    radio.ReadPayload( tempPacketLength, samples + packetOffset8Bits );

    receivedPacketsCount++; // Increase the counter of packets we actually received
    packetOffset += 32UL << 8;
  }

if ( FIFOCount > 1 )
  debugText = str( "FIFO Count = %d", U16(FIFOCount) );

//  radio.FlushRX();

  digitalWrite( PIN_LED_RED, receivedPacketsCount & 0x80 );


// Early return to skip the update of the sample index increment...
//radio.ClearInterruptFlags( _BV(RX_DR) );
//return;


//debugText = str( "%d", U16(receivedPacketsCount) );

  // Determine if we did a full cycle?
//  U32 lastPacketOffset = packetOffset + (32UL << 8);                // Offset is just after the last sample we received
  U32 receivedSamplesCount = packetOffset - firstPacketOffset;  // Actual amount of samples we received from the beginning of the current cycle
  if ( receivedSamplesCount >= (256UL << 8) ) {
      // Start a new cycle and adjust increment so we match the transmitter's data rate
      //
      // The idea here is that we have been playing our samples at our own pace but we just determined that we received enough packets to complete a full cycle (i.e. 256 samples, a full buffer was renewed).
      // So:
      //  • We know the time at which we received Sr = 32 * P samples, where P >= 8 is the amount of packets we received
      //  • We know how many samples Sp we played since the last time we filled an entire buffer (i.e. 8 packets were received)
      //
      // We must adjust the samples increment v to match the transmitter rate:
      //  v = Sr / Sp
      //
      U32 samplesCount = sampleIndex - firstPacketSampleIndex;  // Total samples we've played so far for this cycle

      // Compute adjusted samples increment:
      //  • If both receiver and transmitter were perfectly synchronized, both counts would be equal and the increment would be 1
      //  • If receiver is lagging behind transmitter, then we received more samples than we could play and thus the increment is > 1
      //  • If transmitter is lagging behind receiver, then we played more samples than we received and thus the increment is < 1
      //
//      sampleInc *= (receivedSamplesCount << 8) / samplesCount;
      sampleInc = (receivedSamplesCount << 8) / samplesCount;

//if ( debugText == NULL )
//if ( lastCyclePacketOffset < 0x00030000UL )


//debugText = str( "firstPacketOffset = 0x%08lX - packetOffset = 0x%08lX => 0x%08lX / 0x%08lX = 0x%04X", firstPacketOffset, packetOffset, receivedSamplesCount, samplesCount, U16( sampleInc ) );


    // Start next cycle...
//    firstPacketOffset = ~0UL;
    firstPacketOffset = packetOffset;
    firstPacketSampleIndex = sampleIndex;
  }

  // Only clear interrupt flag once we're done!
  radio.ClearInterruptFlags( _BV(RX_DR) );
}

#else // #ifdef USE_INTERRUPTS

void  FastPacketReader() {
  if ( radio.FIFOStatus() & _BV(RX_EMPTY) )
      return; // RX FIFO empty...

  radio.ClearInterruptFlags( _BV(RX_DR) );

//digitalWrite( PIN_LED_RED, 1 );
//return;

  U8  status = radio.Status();
  U8  pipeIndex = (status & 0x0F) >> 1;
  if ( pipeIndex > 5 ) {
    // Inconsistent state where we received a packet but can't tell in which pipe!
ERROR( true, "FastPacketReader", "MERDE!" );
    radio.FlushRX();
    return;
  }

  // Read payload at offset
  radio.ReadPayload( samples + globalPacketOffset );
  globalPacketOffset += 32U;

  receivedPacketsCount++;
  digitalWrite( PIN_LED_RED, receivedPacketsCount & 0x80 );
}

#endif  // #ifdef USE_INTERRUPTS
