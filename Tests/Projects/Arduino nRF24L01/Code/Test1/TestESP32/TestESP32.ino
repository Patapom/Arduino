////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Receiver Module
//  • Uses IRQ pin to monitor received packets
//  • Uses a 16KHz timer to replay the samples received from the radio packets
//
// ESP32 Subtleties:
//  • GPIO pins 34, 35, 36, 39 are INPUT ONLY!!!
//      => Why are they called GPIO then???
//  • attachInterrupt( PIN, LOW ) does *NOT* work!
//      => Only HIGH, RISING and FALLING seem to be effective
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"
#include <nRF24L01.h>
#include "mynRF24L01.h"
//#include "MCP4725.h"

#define PIN_DAC1 25

char* str::ms_globalBuffer = s_globalBuffer;

nRF24L01  radio( PIN_CE, PIN_CSN );

hw_timer_t* timer0Config = NULL;

void IRAM_ATTR  Timer0_ISR();
void IRAM_ATTR  RadioIRQHandler();

void setup() {
  pinMode( PIN_LED_RED, OUTPUT );
  pinMode( PIN_IRQ, INPUT_PULLUP );
//  pinMode( PIN_IRQ, INPUT );

  Serial.begin( 115200 );
  while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

  Serial.println();
  Serial.println( str( "Clock Frequency = %d.%03d MHz", U16( F_CPU / 1000000L ), U16( (F_CPU / 1000L) % 1000UL ) ) );

/*
  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);
*/

  radio.Init( radio.PIPE0 );  // Only enable data pipe 0
  radio.SetAirDataRateAndPower( radio.DR_2MBPS, radio.PL_0dBm );
  U8  addr[] = { 0xE6, 0xE7, 0xE8, 0xE9, 0xEA };  // Must be the same as transmitter so transmitter packets are forwarded to data pipe 0
  radio.SetReceiverSingleAddress( addr );

  // Enable PRX
  radio.PRX();
//  attachInterrupt( digitalPinToInterrupt( PIN_IRQ ), RadioIRQHandler, LOW );
//  attachInterrupt( PIN_IRQ, RadioIRQHandler, LOW );
  attachInterrupt( PIN_IRQ, RadioIRQHandler, FALLING );
//  attachInterrupt( PIN_IRQ, RadioIRQHandler, HIGH );
  radio.Enable();

//  Serial.println();
//  radio.DumpRegisters();

  Serial.println( "Radio enabled..." );


  timer0Config = timerBegin( 32000 ); // 32KHz
  timerAttachInterrupt( timer0Config, &Timer0_ISR );
//  timerAlarm( timer0Config, 50000, true, reload count?? );
  timerAlarm( timer0Config, 1, true, 0 ); // Signal every time counter reaches 1, i.e. use the full 32KHz frequency of the timer...
  timerRestart( timer0Config );

  Serial.println( str( "Timer enabled..." ) );
}

U16 currentSample = 0;
U32 receivedPacketsCount = 0;

volatile U32  sampleInc = 0x100UL;      // The sample increment, with 8 bits of fixed decimal precision

char* debugText = NULL;

U32 lastMillis = 0;

void loop() {
    
  U32 now = millis();
  if ( (now - lastMillis) > 1000 ) {
    debugText = str( "Inc = 0x%08lX", sampleInc );
    lastMillis = now;
  }

  if ( debugText != NULL ) {
    Serial.println( debugText );
    debugText = NULL;
  }

/*  U8  pipeIndex;
  if ( (pipeIndex = radio.PacketReceived()) != 0xFF ) {
    Serial.println( str( "Packet received on pipe 0x%02X", pipeIndex ) );
  }
*/

//  Serial.println( str( "NRF_STATUS = 0x%02X", radio.Status() ) );
//  Serial.println( str( "NRF_STATUS = 0x%02X", radio.FIFOStatus() ) );
//  delay( 1000 );

//  U8  pipeIndex = radio.PacketReceived();
//  if ( pipeIndex < 6 ) {
//    digitalWrite( PIN_LED_RED, 1 );
//  }

}

// --------------------------------------------------------
// Timer IRQ
//
volatile U32  sampleIndex = 128UL << 8; // The current sample index, with 8 bits of fixed decimal precision
U8            samples[256];             // The tiny circular buffer of samples

// Timer interrupt routine
void IRAM_ATTR  Timer0_ISR() {
//  U8  sampleIndex8Bits = ((U8*) &sampleIndex)[2];
  U8  sampleIndex8Bits = U8( sampleIndex >> 8 );  // Probably cheaper
//  currentSample = samples[sampleIndex8Bits] << 4; // DAC expects 12-bits samples, we only have 8-bits ones...
  currentSample = samples[sampleIndex8Bits];  // ESP32 DAC expects 8-bits samples...

  sampleIndex += sampleInc;


//currentSample = U8( 127.5 + 127.5 * sin( sampleIndex * 2*3.1415926 / (16.0 * 256) ) );  // Generate a 1KHz sine wave


  dacWrite( PIN_DAC1, currentSample );

//  digitalWrite( PIN_LED_RED, (sampleIndex & 0x100000UL) != 0 ); // Blinks ~4 times per second

// DON'T!! Causes guru meditation + reboot
//  Serial.print( "+" );
}

// --------------------------------------------------------
// Radio IRQ
//
U32 firstPacketOffset = ~0UL;
U32 firstPacketSampleIndex = 0;

void IRAM_ATTR  RadioIRQHandler() { // IRAM_ATTR recommended by Espressif to place code in internal RAM instead of flash: much faster execution code

  BaseType_t  xHigherPriorityTaskWoken; // From https://forum.arduino.cc/t/esp32-and-esp-now-for-audio-streaming-slow-acknowledge-from-receiver/1055192


/*digitalWrite( PIN_LED_RED, 1 );
debugText = "Radio IRQ Received!";
radio.ClearInterruptFlags( _BV(RX_DR) );
radio.FlushRX();
return;
*/

  U8  pipeIndex = radio.PacketReceived();
  if ( pipeIndex == 0xFF ) {
    digitalWrite( PIN_LED_RED, 1 );
    return; // Some error occurred! We got notified of an IRQ but flag RX_DR is not set!
  } else if ( pipeIndex == 0xFE ) {
    // Inconsistent state where we received a packet but can't tell in which pipe!
    digitalWrite( PIN_LED_RED, 1 );
    radio.ClearInterruptFlags();
    radio.FlushRX();
    return;
  }

  radio.ClearInterruptFlags( _BV(RX_DR) );

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

    receivedPacketsCount++;
    packetOffset += 32UL << 8;
  }

if ( FIFOCount > 1 )
  debugText = str( "FIFO Count = %d", U16(FIFOCount) );

//  radio.FlushRX();

  // Increase the counter of packets we actually received
  digitalWrite( PIN_LED_RED, receivedPacketsCount & 0x80 );

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
}
