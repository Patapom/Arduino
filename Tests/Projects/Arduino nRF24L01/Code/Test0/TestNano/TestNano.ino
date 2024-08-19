////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Receiver Module
// Doesn't use IRQ pin, continuously monitors the status register for a RX_DR and reads the payload on arrival...
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"
#include "nRF24L01.h"

static char* str::ms_globalBuffer = s_globalBuffer;

nRF24L01  radio( PIN_CE, PIN_CSN );

void setup() {
  pinMode( PIN_LED_RED, OUTPUT );
  pinMode( PIN_IRQ, INPUT_PULLUP );

  // Initiate serial communication
  Serial.begin( 19200 );
  while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

  Serial.println();

  radio.Init( radio.PIPE0 );  // Only enable data pipe 0
  radio.SetAirDataRateAndPower( radio.DR_2MBPS, radio.PL_0dBm );

  U8  addr[] = { 0xE6, 0xE7, 0xE8, 0xE9, 0xEA };  // Must be the same as transmitter so transmitter packets are forwarded to dat apipe 0
  radio.SetReceiverSingleAddress( addr );

  Serial.println();
  Serial.println( "Radio enabled..." );

  radio.DumpRegisters();

  // Enable PRX
  radio.PRX();
  radio.Enable();
  attachInterrupt( digitalPinToInterrupt( PIN_IRQ ), RadioIRQHandler, LOW );
}

volatile bool igloo = false;
volatile U8   packetLength = 0;
volatile U8   packet[33];

void  RadioIRQHandler() {
  U8  pipeIndex = radio.PacketReceived();
  if ( pipeIndex == 0xFF ) {
    return; // Some error occurred! We got notified of an IRQ but flag RX_DR is not set!
  } else if ( pipeIndex == 0xFE ) {
    // Inconsistent state where we received a packet but can't tell in which pipe!
    radio.ClearInterruptFlags();
    return;
  }

  U8  tempPacketLength;
  radio.ReadPayload( tempPacketLength, packet );
  packetLength = tempPacketLength;

  radio.ClearInterruptFlags( _BV(RX_DR) );
//  radio.FlushRX();

  igloo = !igloo;
  digitalWrite( PIN_LED_RED, igloo );
}

U32 runCounter = 0; // How many cycles did we execute?

void loop() {
  runCounter++;
  delay( 100 );  // Each cycle is 1000ms

//  digitalWrite( PIN_LED_RED, runCounter & 1 );

  if ( packetLength ) {
    Serial.println( str( "Packet received => %s", packet ) );
    packetLength = 0;
  }

/*
  U8  pipeIndex, payloadLength;
  U8  payload[33];

  nRF24L01::RECEIVE_RESULT  result = radio.ReceivePacketWait( pipeIndex, payloadLength, payload, 10 );
  switch ( result ) {
      case radio.RR_OK:
        payload[payloadLength] = '\0';
        Serial.println( str( "SUCCESS! Payload on pipe %d, length %d = %s", pipeIndex, payloadLength, payload ) );
        break;

      case radio.RR_INCONSISTENT_FIFO_STATE: // The FIFO is in an inconsistent state: we were notified a packet arrived but the data pipe index is invalid...
        Serial.println( str( "Inconsistent FIFO state! (Status = 0x%02X, FIFO Status = 0x%02X)", radio.Status(), radio.FIFOStatus() ) );
        break;

//      case radio.RR_TIMEOUT:   // Failed to receive packet before elapsed time
//        Serial.println( "Timeout!" );
//        break;
  }
//*/

/*
  // Test SR04
  SetupPins_HCSR04( 6, 5 );

  float distance = MeasureDistance( 6, 5 );
  Serial.println( str( "%d mm", int(1000 * distance) ) );

//  U32 rawTime = MeasureEchoTime( 6, 5 );
//  Serial.println( str( "%d", rawTime ) );

//*/
}
