
// Serial printf from https://arduino.stackexchange.com/users/65/asheeshr
// https://arduino.stackexchange.com/questions/176/how-do-i-print-multiple-variables-in-a-string
//
// Modified by Pom for consistency and '\n' handling
//
#ifndef ARDBUFFER
#define ARDBUFFER 16
#endif

U32	SerialPrintf( char *str, ... );