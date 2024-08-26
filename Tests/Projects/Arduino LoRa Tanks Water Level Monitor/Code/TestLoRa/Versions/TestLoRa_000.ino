#include <SoftwareSerial.h>

#define PIN_RX    2 // RX on D2
#define PIN_TX    3 // TX on D3
#define PIN_RESET 4
#define PIN_LED   5

SoftwareSerial  LoRa( PIN_RX, PIN_TX );

void  SendCommand( String _command ) {
  LoRa.print( _command + "\r\n" );
//  delay( 100 );
}

char  response[128];
void  WaitResponse() {
  char* p = response;
  char  receivedChar = '\0';
  while ( receivedChar != '\n' ) {
    while ( LoRa.available() == 0 ); // Wait until a character is available...
    receivedChar = LoRa.read();
    *p++ = receivedChar;  // Append characters to the received message
Serial.print( receivedChar );
  }
Serial.println( "Received response!" );
  Serial.println( response );  // Print the response to the Serial monitor
}

void Flash( int _duration_ms=1000 ) {
  digitalWrite( PIN_LED, HIGH );
  delay( _duration_ms );
  digitalWrite( PIN_LED, LOW );
}

void setup() {
  pinMode( PIN_LED, OUTPUT );

/*digitalWrite( PIN_TX, LOW );
delay( 1000 );
digitalWrite( PIN_TX, HIGH );
delay( 1000 );
digitalWrite( PIN_TX, LOW );
return;

  // Reset LoRa module
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

delay( 1000 );
//digitalWrite( PIN_LED, HIGH );

Serial.println();
Serial.print( "LoRa is listening? " );
Serial.println( LoRa.isListening() ? "YES" : "NO" );
Serial.println( "Sending commands!" );
delay( 1000 );


//delay( 5000 );
//LoRa.print( "AT\r\n" );
//LoRa.print( "AT+IPR?\r\n" );
//  WaitResponse();

//Serial.println( "BOO!" );
//Flash();


//  SendCommand( "AT+RESET" );  // Normally useless due to hard reset above
//  SendCommand( "AT+IPR=57600" );
//LoRa.print( "AT+IPR?\r\n" );
//  SendCommand( "AT+IPR=9600" );
//  WaitResponse();
//  SendCommand( "AT" );
//LoRa.print( "AT\r\n" );
LoRa.print( "AT+IPR?\r\n" );

bool  receivedStuff = false;
while ( true ) {
//  if ( LoRa.available() ) {
//    Serial.println( String( "Received char = " + LoRa.read() ) );
//  }
    WaitResponse();
    
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
/*
while ( LoRa.available() == 0 ) {
  delay( 10 );  // Wait until a character is available...
}
Serial.println( String( "Received char = " + LoRa.read() ) );
*/
}

String sendMessage;
String receivedMessage;

// Commands (from LoRa AT Command.pdf):
//  AT+RESET
//  AT+IPR=<rate>             // Set baud rate (default 57600)
//  AT+ADDRESS=<ID 16 bits>   // Specific to the module (default 0)
//  AT+NETWORKID=[3,15]       // Common to all modules (default 18)
//  AT+BAND=915000000         // Set the center frequency of wireless band. Common to all modules (default 915000000)
//  AT+PARAMETER= 9,7,1,12   
//                              [1] <Spreading Factor>: The larger the SF is, the better the sensitivity is. But the transmission time will take longer. 5~11 (default 9) *SF7to SF9 at 125kHz, SF7 to SF10 at 250kHz, and SF7 to SF11 at 500kHz
//                              [2] <Bandwidth>: The smaller the bandwidth is, the better the sensitivity is. But the transmission time will take longer. 7: 125 KHz (default), 8: 250 KHz, 9: 500 KHz
//                              [3] <Coding Rate>: The coding rate will be the fastest if setting it as 1.
//                              [4] <Programmed Preamble>: Preamble code. If the preamble code is bigger, it will result in the less opportunity of losing data.
//                                    Generally preamble code can be set above 10 if under the permission of the transmission time.
//                                    When the Payload length is greater than 100 bytes, recommend to set “AT + PARAMETER = 8,7,1,12”
//  AT+CPIN=<Password>        // Domain password (4 bytes hexa)
//  AT+CRFOP=<power [0,22]>   // RF Output power in dBm (default=22)
//  AT+SEND=<address 16 bits>, <payload size [0,240]>, <payload>  // Due to the program used by the module, the payload part will increase more 8 bytes than the actual data length.


void loop() {
if ( LoRa.available() ) {
  Serial.println( String( "Received char = " + LoRa.read() ) );
}

return;
//  Serial.println("Hello world!");
//*
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

//*/
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
}
