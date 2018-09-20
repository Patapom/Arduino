#include "../../Pom.h"

// TODO: Check power-down bits
//const bool	BIT_PD0 = 0;
//const bool	BIT_PD1 = 0;

void	MCP4725_Begin( TWI& _twi, U8 _MCPAddress ) {
	_twi.BeginTransmit( _MCPAddress );
}

void	MCP4725_SetValue( TWI& _twi, U16 _value ) {
	_value &= 0x0FFF;
//	_value |= (BIT_PD0 << 12) | (BIT_PD1 << 13);	// Use this if PD0/PD1 are not 0
	SwapBytes( _value );
	_twi.Push( (U8*) &_value, 2 );
}
void	MCP4725_SetValueEEPROM( TWI& _twi, U16 _value ) {
	_value &= 0x0FFF;
	U8	bytes[3];
	bytes[0] = 0x60;	// Write DAC + EEPROM
//	bytes[0] = 0x60 | (BIT_PD0 << 1) | (BIT_PD1 << 2);	// Use this if PD0/PD1 are not 0
	bytes[1] = _value >> 4;
	bytes[2] = U8( _value << 4);
	_twi.Push( bytes, 3 );
}
