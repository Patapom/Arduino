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
  pinMode( PIN_LED_RED, OUTPUT );
  pinMode( PIN_LED_GREEN, OUTPUT );

  // Setup the HC-SR04
  SetupPins_HCSR04( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );

  // Initiate serial communication
  Serial.begin( 19200 );        // This one is connected to the PC
  while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

  Flash( PIN_LED_GREEN, 50, 10 );

/*  Test LEDS
while ( true ) {
  Flash( PIN_LED_RED, 100, 10 );
  Flash( PIN_LED_GREEN, 100, 10 );
}
//*/

/* Test SR04
while ( true ) {
//  Serial.println( "Allo?" );
  float distance = MeasureDistance( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );
  Serial.println( String( "distance = " ) + distance );
  delay( 100 );
}
//*/

/*  // Hardware reset LoRa module
  pinMode( PIN_RESET, OUTPUT );
  digitalWrite( PIN_RESET, LOW );
  delay( 200 ); // Advised value is at least 100ms
  digitalWrite( PIN_RESET, HIGH );
*/

  LoRa.begin( LORA_BAUD_RATE ); // This software serial is connected to the LoRa module

/* Software reset takes an annoyingly long time...
  SendCommandAndWaitPrint( "AT+RESET" );  // Normally useless due to hard reset above
  delay( 5000 );
//*/

  #ifdef DEBUG_LIGHT
    Serial.println();
//    Serial.print( "LoRa is listening? " );
//    Serial.println( LoRa.isListening() ? "YES" : "NO" );
    Serial.println( "Initializing..." );
//delay( 1000 );
  #endif

  // Initialize LoRa module
  #ifdef TRANSMITTER
    CONFIG_RESULT configResult = ConfigureLoRaModule( NETWORK_ID, TRANSMITTER );
  #else
    CONFIG_RESULT configResult = ConfigureLoRaModule( NETWORK_ID, RECEIVER_ADDRESS );
  #endif

  #ifdef DEBUG_LIGHT
    Serial.println( configResult != CR_OK ? "Configuration failed with code " + String( (int) configResult ) : "Configuration successful!" );
  #endif

  if ( configResult != CR_OK ) {
    // In error...
    while ( true ) {
      Flash( PIN_LED_RED, 250, 1 );
    }
  }

  // Enable "smart mode"
  #if defined(TRANSMITTER) && defined(USE_SMART_MODE)
    result = SetWorkingMode( WM_SMART, 1000, 10000 ); // Sleep for 10 seconds, active for 1 second
    if ( result != CR_OK )
      Serial.println( "Smart mode failed with code " + String( (int) result ) );
    else
      Serial.println( "Smart mode successful!" );
  #endif

// @TODO: Password encryption!
// Optional password
//  SendCommandAndWaitPrint( "AT+CPIN?" );
//  ClearPassword();
//  SetPassword( 0x1234ABCDU );
//  SendCommandAndWaitPrint( "AT+CPIN?" );
}

U32 runCounter = 0; // How many cycles did we execute?

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SERVER SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#ifndef TRANSMITTER

U32   cyclesCounter = ~0U;
U16   commandID = 0;
bool  commandSent = false;
char* response;
U16   senderAddress;
U8    payloadLength;

void loop() {
  runCounter++;
  delay( 100 );  // Each cycle is 100ms
//  digitalWrite( PIN_LED_GREEN, runCounter & 1 );

  cyclesCounter++;
  if ( cyclesCounter >= 25 ) {
    // Ask client 1 to measure a distance every 25 cycles (i.e. every 2.5 seconds)...
    Serial.println( String( "Client 1 => Execute DST0," ) + String( commandID, 16 ) + "..." );
    Flash( 250 );
    Execute( 1, "DST0", commandID++, 0, "" );
    commandSent =  true;

    cyclesCounter = 0;
  }

if ( commandSent ) {
  Serial.print( "." );  // Usually a reply comes in after "........" dots, a missed command gives ".................................................."
// @TODO: Resend command after too many dots (failed command)
// @TODO: Notify of module in error state after too many failed commands
}

  // Check for a reply
  RECEIVE_RESULT  RR = ReceivePeek( senderAddress, payloadLength, response );
//  RECEIVE_RESULT  RR = ReceiveWait( senderAddress, payloadLength, response );
//  Serial.println( String( "Receive Result = " ) + RR );
  if ( RR != RR_OK ) {
    return; // No reply...
  }

  Serial.println( String( "Reply from client " ) + senderAddress );
  Serial.println( String( "Payload Length = " ) + payloadLength );
  Serial.println( String( "Payload = " ) + response );
  Serial.println();

  if ( payloadLength < 10 || !HandleReply( senderAddress, payloadLength, response ) ) {
    Serial.println( "Failed to handle reply..." );
  }

  commandSent = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLIENT SIDE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#else

void loop() {
  runCounter++; // 1 more cycle...
  #ifdef DEBUG
    digitalWrite( PIN_LED_GREEN, runCounter & 1 );
  #endif

  // Check for a command
  U16   senderAddress;
  U8    payloadLength;
  char* payload;
  U8    result;
  if ( (result = ReceivePeek( senderAddress, payloadLength, payload )) != RR_OK ) {
//  if ( (result = ReceiveWait( senderAddress, payloadLength, payload )) != RR_OK ) {
    // Nothing received...
    delay( CLIENT_POLL_INTERVAL_MS );
    return;
  }

  #ifdef DEBUG_LIGHT
    Serial.println( String( "Client 1 => Received " ) + payload );
  #endif

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
  } else if ( QuickCheckCommand( payload, "PING" ) ) {
    ExecuteCommand_Ping( payloadLength, payload );
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
  Reply( _payload, _payload+5, String( runCounter ) );  // Send back the runtime counter
}

void  ExecuteCommand_Ping(  U8 _payloadLength, char* _payload ) {
  Reply( _payload, _payload+5, "" );  // Just send the ping back...
}

#endif