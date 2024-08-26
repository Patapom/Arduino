#include "Global.h"

// Define this to be the receiver module, undefine to be the transmitter module
#define RECEIVER

SoftwareSerial  LoRa( PIN_RX, PIN_TX );

void setup() {
  pinMode( PIN_LED, OUTPUT );

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
Serial.println( "Initializeing..." );
//delay( 1000 );

/* Reset takes an annoyingly long time...
  SendCommandAndWaitPrint( "AT+RESET" );  // Normally useless due to hard reset above
  delay( 5000 );
*/

//SendCommandAndWaitPrint( "AT+RESET" );

//  SendCommand( "AT+IPR=57600" );
//LoRa.print( "AT+IPR?\r\n" );
//  SendCommand( "AT+IPR=9600" );
//  WaitResponse();
//  SendCommand( "AT" );
//  SendCommand( "AT+IPR?" );
//LoRa.print( "AT\r\n" );
//LoRa.print( "AT+IPR?\r\n" );

  #ifdef RECEIVER
    CONFIG_RESULT result = ConfigureLoRaModule( NETWORK_ID, RECEIVER_ADDRESS );
  #else
    CONFIG_RESULT result = ConfigureLoRaModule( NETWORK_ID, TRANSMITTER_ADDRESS );
  #endif
  if ( result != CR_OK )
    Serial.println( "Configuration failed with code " + String( (int) result ) );
  else
    Serial.println( "Configuration successful!" );

// Optional password
//SendCommandAndWaitPrint( "AT+CPIN?" );
//  ClearPassword();
//  SetPassword( 0x1234ABCDU );
//  SetPassword( 0x10101010U );

//U8 strangeCharacters[] = { 0x00, 0x01, 0x80, 0xFF, 0x64 };
//char  strangeCharacters[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 0x80, 0x81 };  // 34 characters
//char  strangeCharacters[] = { 0xFF, 0xFE, 0xFD, 0xFC, 0xF0, 0xE0, 0xD0, 0xC0, 0xB0, 0xA0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10 }; // 19 characters
//if ( Send( 1, 19, strangeCharacters ) != SR_OK )

//if ( Send( 1, 5, "HELLO" ) != SR_OK )
//  Serial.println( "Error sending message!" );

//  if ( LoRa.available() ) {
//    Serial.println( String( "Received char = " + LoRa.read() ) );
//  }
//    WaitResponse();
    
// This checks that we can immediately run some code to listen to the reply...
//  delay(1);
//  digitalWrite( PIN_TX, LOW );
//  delay(1);
//  digitalWrite( PIN_TX, HIGH );

  // This checks we actually receive a reply from the LoRa module! And it works! So WTF?
//  receivedStuff |= !digitalRead( PIN_RX );
//  if ( receivedStuff )
//    digitalWrite( PIN_LED, HIGH );
}

#ifdef RECEIVER
void loop() {
  char* response;
//  response = WaitResponse();
//  if ( response != NULL ) {
//    Serial.println( response );
//  }

  U16 address;
  U8  payloadLength;
  if ( WaitReceive( address, payloadLength, response ) == RR_OK ) {
    #ifdef DEBUG
      Serial.println( response );
    #endif

    if ( String( response ) == "HELLO" )
      digitalWrite( PIN_LED, HIGH );
    else if ( String( response ) == "WORLD" )
      digitalWrite( PIN_LED, LOW );
    else
      Flash( 50, 10 ); // Unknown response!
  }
}

#else
String sendMessage;
String receivedMessage;

bool  error = false;
bool  sendHello = false;

void loop() {

  sendHello = !sendHello;
  if ( error ) {
    digitalWrite( PIN_LED, sendHello );
    delay( 150 );
    return;
  }

  if ( Send( RECEIVER_ADDRESS, 5, sendHello ? "HELLO" : "WORLD" ) != SR_OK )
    error = true; // Argh!
  digitalWrite( PIN_LED, sendHello );
  delay( 1000 );

/*if ( LoRa.available() ) {
  Serial.println( String( "Received char = " + LoRa.read() ) );
}

return;
//  Serial.println("Hello world!");

  while (LoRa.available() > 0) {
    char receivedChar = LoRa.read();
    if (receivedChar == '\n') {
      Serial.println(receivedMessage);  // Print the received message in the Serial monitor
      receivedMessage = "";  // Reset the received message
    } else {
      receivedMessage += receivedChar;  // Append characters to the received message
    }
  }

  LoRa.print( "AT\r\n" );
  delay(1000);
  return;

 if (Serial.available() > 0) {
    char inputChar = Serial.read();
    if ( inputChar == '\n' ) {
      LoRa.println(sendMessage);  // Send the message through LoRa with a newline character
      Serial.println( "Received command " + sendMessage + ". Sent to LoRa." );
      sendMessage = "";  // Reset the message
    } else {
      sendMessage += inputChar;  // Append characters to the message
    }
  }
*/  
}
#endif