#include "Global.h"

// Define this to be the receiver module, undefine to be the transmitter module
#define RECEIVER

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

  #ifdef RECEIVER
    CONFIG_RESULT result = ConfigureLoRaModule( NETWORK_ID, RECEIVER_ADDRESS );
  #else
    CONFIG_RESULT result = ConfigureLoRaModule( NETWORK_ID, TRANSMITTER_ADDRESS );
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

#ifdef RECEIVER
void loop() {
  char* response;
//  response = WaitResponse();
//  if ( response != NULL ) {
//    Serial.println( response );
//  }

  U16 address;
  U8  payloadLength;
  RECEIVE_RESULT  RR = ReceivePeek( address, payloadLength, response );
//  RECEIVE_RESULT  RR = ReceiveWait( address, payloadLength, response );
//  Serial.println( String( "Receive Result = " ) + RR );
  if ( RR == RR_OK ) {
    Serial.println( String( "Address = " ) + address );
    Serial.println( String( "Payload Length = " ) + payloadLength );
    Serial.println( String( "Payload = " ) + response );
  }

/*  U16 address;
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
*/  
}

#elif 1
U32 runCounter = 0;
void loop() {
  // Measure distance
  U32 timeOfFlight_microSeconds = MeasureEchoTime( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );
  String  message;
  if ( timeOfFlight_microSeconds < 38000 ) {
    message = "Echo0:";
    message += timeOfFlight_microSeconds;
  } else {
    message = "Echo0:OOR";  // Out of range error!
  }

  // Send the message
  if ( Send( RECEIVER_ADDRESS, message ) != SR_OK ) {
    Flash( 50, 10 );  // Error!
  }

// @TODO: Delay should be adjustable!
  delay( 1000 );

  // 1 more cycle...
  runCounter++;
  digitalWrite( PIN_LED, runCounter & 1 );
}

#else // Test version that sends "HELLO" and "WORLD" in a cycle
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

  // Measure distance using HC-SR04
  #if 1
    // Show raw time of flight
    U32 timeOfFlight_microSeconds = MeasureEchoTime( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );
    if ( timeOfFlight_microSeconds > 38000 )
      Serial.println( "Out of range!" );
    else
      Serial.println( String( "Time of flight = " ) + timeOfFlight_microSeconds + " µs" );
  #else
    // Show actual distance in meters
    float distance = MeasureDistance( PIN_HCSR04_TRIGGER, PIN_HCSR04_ECHO );
    if ( distance > 0 )
      Serial.println( String( "Distance = " ) + distance + " m" );
  #endif
}
#endif