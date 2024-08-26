#ifndef GLOBAL_H
#define GLOBAL_H

// Define this to output ALL commands/responses to the serial
//#define DEBUG

// Define this to output end product commands/responses to the serial (e.g. client send/receive, server send/receive)
#define DEBUG_LIGHT

// Define this to be the transmitter module (i.e. CLIENT) and specify the transmitter address, undefine to be the receiver module (i.e. SERVER)
//#define TRANSMITTER 1   // Transmitter address is 1

// Default receiver address is the simplest address
#define RECEIVER_ADDRESS  0

// Define this to use the "smart mode" (a.k.a. mode 2)
//#define USE_SMART_MODE

#ifdef TRANSMITTER
#if TRANSMITTER == RECEIVER_ADDRESS
  "ERROR! Transmitter cannot use the address reserved for the receiver!"
#endif
#endif

#define LORA_BAUD_RATE  19200 // Can't use too high baud rates with software serial!

static const int  CLIENT_POLL_INTERVAL_MS = 100; // Poll for a command every 100 ms

#define _SS_MAX_RX_BUFF 512 // Increase buffer size for software serial so we don't risk overwriting the circular buffer!
#include <SoftwareSerial.h>

#define PIN_RX    2 // RX on D2
#define PIN_TX    3 // TX on D3
//#define PIN_RESET 4
#define PIN_LED_GREEN 4
#define PIN_LED_RED   5

// HC-SR04 Ultrasound Distance Measurement device
#define PIN_HCSR04_TRIGGER  6
#define PIN_HCSR04_ECHO     7

#define NETWORK_ID  5

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
  CR_COMMAND_FAILED_,   // Command failed
  CR_COMMAND_FAILED_AT,             // AT didn't return +OK
  CR_COMMAND_FAILED_AT_NETWORKID,   // AT+NETWORKID didn't return +OK
  CR_COMMAND_FAILED_AT_ADDRESS,     // AT+ADDRESS didn't return +OK
  CR_COMMAND_FAILED_AT_PARAMETER,   // AT+PARAMETER didn't return +OK
  CR_COMMAND_FAILED_AT_CPIN,        // AT+CPIN didn't return +OK
  CR_COMMAND_FAILED_AT_MODE,        // AT+MODE didn't return +OK
  CR_INVALID_PASSWORD,              // INvalid password for AT+CPIN (most likely 0)
};

enum WORKING_MODE {
  WM_TRANSCEIVER = 0, // Transceiver mode (Default)
  WM_SLEEP = 1,       // Sleep mode
  WM_SMART = 2,       // "Smart mode" where the device is in sleep mode for N seconds and in transceiver mode for N' seconds
};

/*
void Flash( int _duration_ms ) {
  Flash( PIN_LED_RED, _duration_ms );
}
void Flash( U8 _pin, int _duration_ms ) {
  digitalWrite( _pin, HIGH );
  delay( _duration_ms );
  digitalWrite( _pin, LOW );
}
//*/
void Flash( int _pin, int _duration_ms, int _count ) {
  for ( int i=0; i < _count; i++ ) {
    digitalWrite( _pin, HIGH );
    delay( _duration_ms );
    digitalWrite( _pin, LOW );
    if ( i != _count-1 )
      delay( _duration_ms );
  }
}
void Flash( int _duration_ms ) {
  Flash( PIN_LED_RED, _duration_ms, 1 );
}
void Flash( int _duration_ms, int _count ) {
  Flash( PIN_LED_RED, _duration_ms, _count );
}

#endif