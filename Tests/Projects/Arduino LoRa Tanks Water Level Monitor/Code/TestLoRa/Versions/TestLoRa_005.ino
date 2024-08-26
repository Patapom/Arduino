////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This code is designed to either act as a server (i.e. RECEIVER) or a client (i.e. TRANSMITTER)
//  • The server side is located on the local PC, it sends commands to the clients which respond in turn
//  • The client side responds to the server's command by executing the commands and sending a response
//    ☼ Typically, a client will host various sensors that can be triggered and/or interrogated on demand by the server
//    ☼ The client-side logic should be minimal: wait for server commands to execute, execute them and send the response...
//
// Switch between compiling a server or a client by defining or not the TRANSMITTER macro in Global.h
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"

SoftwareSerial  LoRa( PIN_RX, PIN_TX );

void setup() {
  pinMode( PIN_LED, OUTPUT );

  // Setup the HC-SR04
  SetupPins_HCSR04( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );

/*  // Reset LoRa module
  pinMode( PIN_RESET, OUTPUT );
  digitalWrite( PIN_RESET, LOW );
  delay( 200 ); // Advised value is at least 100ms
  digitalWrite( PIN_RESET, HIGH );
*/

  // Initiate serial communication
  Serial.begin( 9600 );         // This one is connected to the PC
  while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

//  LoRa.begin( 115200 ); // Can't use too high baud rates with software serial!
  LoRa.begin( 9600 );      // This one is connected to the LoRa module

Serial.println();
Serial.print( "LoRa is listening? " );
Serial.println( LoRa.isListening() ? "YES" : "NO" );
Serial.println( "Initializing..." );
//delay( 1000 );

/* Reset takes an annoyingly long time...
  SendCommandAndWaitPrint( "AT+RESET" );  // Normally useless due to hard reset above
  delay( 5000 );
*/

  #ifdef TRANSMITTER
    CONFIG_RESULT result = ConfigureLoRaModule( NETWORK_ID, TRANSMITTER );
  #else
    CONFIG_RESULT result = ConfigureLoRaModule( NETWORK_ID, RECEIVER_ADDRESS );
  #endif
  if ( result != CR_OK )
    Serial.println( "Configuration failed with code " + String( (int) result ) );
  else
    Serial.println( "Configuration successful!" );

// @TODO: Password encryption!
// Optional password
//SendCommandAndWaitPrint( "AT+CPIN?" );
//  ClearPassword();
//  SetPassword( 0x1234ABCDU );
//  SetPassword( 0x10101010U );
}

U32 runCounter = 0; // How many cycles did we execute?

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#ifndef TRANSMITTER

U32 cyclesCounter = 0;
void loop() {
  runCounter++;
  delay( 100 );  // Each cycle is 100ms
  digitalWrite( PIN_LED, runCounter & 1 );

  cyclesCounter++;
  if ( cyclesCounter == 50 ) {
    // Ask client 1 to measure a distance every 50 cycles (i.e. every 5 seconds)...
    Execute( 1, "DST0", 0, "" );
    cyclesCounter = 0;
  }

  // Check for a reply
  char* response;
  U16 senderAddress;
  U8  payloadLength;
  RECEIVE_RESULT  RR = ReceivePeek( senderAddress, payloadLength, response );
//  RECEIVE_RESULT  RR = ReceiveWait( senderAddress, payloadLength, response );
//  Serial.println( String( "Receive Result = " ) + RR );
  if ( RR != RR_OK ) {
    return; // No reply, or not for us anyway...
  }

  Serial.println( String( "Reply from client " ) + senderAddress );
  Serial.println( String( "Payload Length = " ) + payloadLength );
  Serial.println( String( "Payload = " ) + response );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLIENT SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#else

void loop() {
  runCounter++; // 1 more cycle...
  #ifdef DEBUG
    digitalWrite( PIN_LED, runCounter & 1 );
  #endif

  // Check for a command
  U16   senderAddress;
  U8    payloadLength;
  char* payload;
  if ( ReceivePeek( senderAddress, payloadLength, payload ) != RR_OK ) {
    // Nothing received...
    delay( CLIENT_POLL_INTERVAL_MS );
    return;
  }
  if ( senderAddress != RECEIVER_ADDRESS ) {
    return; // Not from the server...
  }

  if ( payloadLength < 4 ) {
    // Invalid payload!
    Flash( 50, 10 );  // Flash to signal error! => We received something that is badly formatted...
    return;
  }

  // Analyze command
  if ( strstr( payload, "CMD=" ) != payload ) {
    // Not a command?
    Flash( 50, 10 );  // Flash to signal error! => We received something that is badly formatted...
    return;
  }

  // Skip "CMD="
  payload += 4;
  payloadLength -= 4;

  if ( payloadLength < 4 ) {
    // Invalid payload!
    Flash( 50, 10 );  // Flash to signal error! => We received something that is badly formatted...
    return;
  }

  // Check for common commands
  if ( QuickCheckCommand( payload, "TIME" ) ) {
    ExecuteCommand_Runtime( payloadLength, payload );
    return;
  }

  // Let handler check for supported commands
  if ( !HandleCommand( payloadLength, payload ) ) {
    // Unrecognized command?
    Flash( 150, 10 );  // Flash to signal error!
    return;
  }
}

void  ExecuteCommand_Runtime( U8 _payloadLength, char* _payload ) {
  Reply( _payload, String( runCounter ) );
}

#endif