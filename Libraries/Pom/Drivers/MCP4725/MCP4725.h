//////////////////////////////////////////////////////////////////////////
// The MCP4725 is a simple Digital-Analog Converter (DAC) that is driven through the TWI/I2C interface
//////////////////////////////////////////////////////////////////////////
//

// Begins transfer to MCP4725
void	MCP4725_Begin( TWI& _twi, U8 _MCPAddress );

// Sets the value for the MCP4725
//	_value, the 12-bits DAC value in [0,4095]
void	MCP4725_SetValue( TWI& _twi, U16 _value );			// Fast mode, only 2 bytes are transmitted
void	MCP4725_SetValueEEPROM( TWI& _twi, U16 _value );	// Full mode, DAC and EEPROM are written
