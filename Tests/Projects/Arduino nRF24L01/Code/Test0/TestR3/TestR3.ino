////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transmitter Module
// Doesn't use IRQ pin, sends a payload and continuously waits until a TX_DS or MAX_RT bit is set before returning success or failure...
//
// The surprising thing with this code is that it quite often returns  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"
#include "nRF24L01.h"
#include <RF24.h>

static char* str::ms_globalBuffer = s_globalBuffer;

nRF24L01  radio( PIN_CE, PIN_CSN );

void  setup() {
  pinMode( PIN_LED_RED, OUTPUT );

  // Initiate serial communication
  Serial.begin( 19200 );
  while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

  Serial.println();
  radio.Init( radio.PIPE0 );  // Only enable data pipe 0
  radio.SetAirDataRateAndPower( radio.DR_2MBPS, radio.PL_0dBm );

  radio.PTX();
  U8  addr[] = { 0xE6, 0xE7, 0xE8, 0xE9, 0xEA };
  radio.SetTransmitterAddress( addr );

  radio.DumpRegisters();
}

U32 runCounter = 0; // How many cycles did we execute?

void  loop() {
  runCounter++;
  delay( 1000 );  // Each cycle is 1000ms

  char  coucou[33];
  int payloadLength = sprintf( coucou, "COUCOU WORLD %d!", runCounter );

  Serial.print( str( "%s -> ", coucou ) );

  nRF24L01::TRANSMIT_RESULT result = radio.SendPacket( payloadLength, coucou );
//  Serial.println( str( "Transmit result = 0x%02X", result ) );

  switch ( result ) {
    case radio.TR_OK:
    Serial.println( "SUCCESS!" );
//radio.ClearMAX_RT();
//radio.FlushTX();
    break;

    case radio.TR_FAILED:
      Serial.println( "FAILED! Flushing TX..." );
      break;

    case radio.TR_FIFO_EMPTY:
     Serial.println( "FIFO empty???" );
     break;
    
    case radio.TR_FIFO_FULL:
      Serial.println( "FIFO full??? Flushing TX..." );
//radio.ClearMAX_RT();
      radio.FlushTX();
      break;
  }
}
