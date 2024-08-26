#ifndef GLOBAL_H
#define GLOBAL_H

//#define DEBUG // Define this to output commands/response to the serial

#include <SoftwareSerial.h>

#define PIN_RX    2 // RX on D2
#define PIN_TX    3 // TX on D3
#define PIN_RESET 4
#define PIN_LED   5

// HC-SR04 Ultrasound Distance Measurement device
#define PIN_HCSR04_TRIGGER  6
#define PIN_HCSR04_ECHO     7

#define NETWORK_ID  5
#define TRANSMITTER_ADDRESS 0
#define RECEIVER_ADDRESS 1

typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   byte;
typedef unsigned long   U32;
typedef unsigned short  U16;
typedef unsigned char   U8;

enum RESPONSE_TYPE {
  RT_OK = 0,      // Got the expected response
  RT_ERROR = 1,   // Wrong response
  RT_TIMEOUT = 2, // The command timed out
};

enum SEND_RESULT {
  SR_OK = 0,      // Success!
  SR_INVALID_PAYLOAD_SIZE,
  SR_INVALID_PAYLOAD,
  SR_TIMEOUT,     // Send returned a timeout (command failed to return a response)
  SR_ERROR,       // Send returned an error!
};

enum RECEIVE_RESULT {
  RR_OK = 0,      // Success!
  RR_EMPTY_BUFFER,// Notifies an empty reception buffer when doing active receive peeking using PeekReceive()
  RR_TIMEOUT,     // Receive returned a timeout (command failed to return a response)
  RR_ERROR,       // Receive returned an error!
};

enum CONFIG_RESULT {
  CR_OK = 0,            // Success!
  CR_INVALID_PARAMETER, // One of the parameters is not in the appropriate range!
  CR_COMMAND_FAILED_,    // Command failed
  CR_COMMAND_FAILED_AT,             // AT didn't return +OK
  CR_COMMAND_FAILED_AT_NETWORKID,   // AT+NETWORKID didn't return +OK
  CR_COMMAND_FAILED_AT_ADDRESS,     // AT+ADDRESS didn't return +OK
  CR_COMMAND_FAILED_AT_PARAMETER,   // AT+PARAMETER didn't return +OK
  CR_COMMAND_FAILED_AT_CPIN,        // AT+CPIN didn't return +OK
  CR_INVALID_PASSWORD,              // INvalid password for AT+CPIN (most likely 0)
};

void Flash( int _duration_ms=1000 ) {
  digitalWrite( PIN_LED, HIGH );
  delay( _duration_ms );
  digitalWrite( PIN_LED, LOW );
}
void Flash( int _duration_ms=1000, int _count=1 ) {
  for ( int i=0; i < _count; i++ ) {
    digitalWrite( PIN_LED, HIGH );
    delay( _duration_ms );
    digitalWrite( PIN_LED, LOW );
    if ( i != _count-1 )
      delay( _duration_ms );
  }
}

#endif