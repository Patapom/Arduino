#include "Arduino.h"
#include "CC1101.h"

#include "Pom/Pom.h"

using namespace Pom;

void	delayns( uint32_t _nanoseconds ) {
	uint32_t	microseconds = (999 + _nanoseconds) / 1000;	// We don't have enough resolution for nanoseconds here so we round up to the next microsecond
	delayMicroseconds( microseconds );
}

CC1101::CC1101( byte _CS, byte _CLOCK, byte _SI, byte _SO, byte _GDO0, byte _GDO2 ) {
	// Setup pins
	m_pin_CS = _CS;
	m_pin_Clock = _CLOCK;
	m_pin_SI = _SI;
	m_pin_SO = _SO;
	m_pin_GDO0 = _GDO0;
	m_pin_GDO2 = _GDO2;

	pinMode( m_pin_Clock, OUTPUT );
	pinMode( m_pin_CS, OUTPUT );
	pinMode( m_pin_SO, INPUT );		// Master In Slave Out
	pinMode( m_pin_SI, OUTPUT );	// Master Out Slave In
// 	pinMode( m_pin_GDO0, INPUT );
// 	pinMode( m_pin_GDO2, INPUT );

	digitalWrite( m_pin_CS, HIGH );	// Clear the line

	// Initialize registers
// 	SPITransfer( OP_SCANLIMIT, 7 );		// Scanlimit is set to max on startup
// 	SPITransfer( OP_DECODEMODE, 0 );	// No decode: we send raw bits!
// 	SPITransfer( OP_INTENSITY, 16 );	// Max intensity!
// 	SPITransfer( OP_DISPLAYTEST, 0 );	// Stop any current test
// 	SPITransfer( OP_SHUTDOWN, 1 );		// Wake up!
}

enum MODULATION_FORMAT {
// 0 (000) 2-FSK
// 1 (001) GFSK
// 2 (010) -
// 3 (011) ASK/OOK
// 4 (100) 4-FSK
// 5 (101) -
// 6 (110) -
// 7 (111) MSK
};

enum SYNC_MODE {
// 0 (000) No preamble/sync
// 1 (001) 15/16 sync word bits detected
// 2 (010) 16/16 sync word bits detected
// 3 (011) 30/32 sync word bits detected
// 4 (100) No preamble/sync, carrier-sense
// above threshold
// 5 (101) 15/16 + carrier-sense above threshold
// 6 (110) 16/16 + carrier-sense above threshold
// 7 (111) 30/32 + carrier-sense above threshold
};

// MDMCFG2.SYNC_MODE Sync Word Qualifier Mode
enum SYNC_WORD_QUALIFIER_MODE {
// 000 No preamble/sync
// 001 15/16 sync word bits detected
// 010 16/16 sync word bits detected
// 011 30/32 sync word bits detected
// 100 No preamble/sync + carrier sense above threshold
// 101 15/16 + carrier sense above threshold
// 110 16/16 + carrier sense above threshold
// 111 30/32 + carrier sense above threshold
};

// Main Radio Control FSM State
enum MARC_STATE {
	SLEEP				= 0x00,
	IDLE				= 0x01,
	XOFF				= 0x02,
	VCOON_MC			= 0x03,
	REGON_MC			= 0x04,
	MANCAL				= 0x05,
	VCOON				= 0x06,
	REGON				= 0x07,
	STARTCAL			= 0x08,
	BWBOOST				= 0x09,
	FS_LOCK				= 0x0A,
	IFADCON				= 0x0B,
	ENDCAL				= 0x0C,
	RX					= 0x0D,
	RX_END				= 0x0E,
	RX_RST				= 0x0F,
	TXRX_SWITCH			= 0x10,
	RXFIFO_OVERFLOW		= 0x11,
	FSTXON				= 0x12,
	TX					= 0x13,
	TX_END				= 0x14,
	RXTX_SWITCH			= 0x15,
	TXFIFO_UNDERFLOW	= 0x16,
};

enum REGISTERS {
	IOCFG2	  =	0x00,
	IOCFG1	  =	0x01,
	IOCFG0	  =	0x02,
	FIFOTHR	  =	0x03,
	SYNC1	  =	0x04,
	SYNC0	  =	0x05,
	PKTLEN	  =	0x06,
	PKTCTRL0  =	0x07,
	PKTCTRL1  =	0x08,
	ADDR	  =	0x09,
	CHANNR	  =	0x0A,
	FSCTRL1	  =	0x0B,
	FSCTRL0	  =	0x0C,
	FREQ2	  =	0x0D,
	FREQ1	  =	0x0E,
	FREQ0	  =	0x0F,
	MDMCFG4	  =	0x10,
	MDMCFG3	  =	0x11,
	MDMCFG2	  =	0x12,
	MDMCFG1	  =	0x13,
	MDMCFG0	  =	0x14,
	DEVIATN	  =	0x15,
	MCSM2	  =	0x16,
	MCSM1	  =	0x17,
	MCSM0	  =	0x18,
	FOCCFG	  =	0x19,
	BSCFG	  =	0x1A,
	AGCTRL2	  =	0x1B,
	AGCTRL1	  =	0x1C,
	AGCTRL0	  =	0x1D,
	WOREVT1	  =	0x1E,
	WOREVT0	  =	0x1F,
	WORCTRL	  =	0x20,
	FREND1	  =	0x21,
	FREND0	  =	0x22,
	FSCAL3	  =	0x23,
	FSCAL2	  =	0x24,
	FSCAL1	  =	0x25,
	FSCAL0	  =	0x26,
	RCCTRL1	  =	0x27,
	RCCTRL0	  =	0x28,
	FSTEST	  =	0x29,
	PTEST	  =	0x2A,
	AGCTEST	  =	0x2B,
	TEST2	  =	0x2C,
	TEST1	  =	0x2D,
	TEST0	  =	0x2E,
};

enum COMMAND_STROBES {
	SRES		= 0x30,	// Reset chip.
	SFSTXON		= 0x31,	// Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX (with CCA): Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
	SXOFF		= 0x32,	// Turn off crystal oscillator.
	SCAL		= 0x33,	// Calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without setting manual calibration mode (MCSM0.FS_AUTOCAL=0)
	SRX			= 0x34,	// Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1.
	STX			= 0x35,	// In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear.
	SIDLE		= 0x36,	// Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable.
	SWOR		= 0x38,	// Start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if WORCTRL.RC_PD=0.
	SPWD		= 0x39,	// Enter power down mode when CSn goes high.
	SFRX		= 0x3A,	// Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states.
	SFTX		= 0x3B,	// Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.
	SWORRST		= 0x3C,	// Reset real time clock to Event1 value.
	SNOP		= 0x3D,	// No operation. May be used to get access to the chip status byte.
};

byte	CC1101::ReadStatus() {
	return 0;	// TODO!
}

void	CC1101::Reset() {
	// Set SCLK = 1 and SI = 0, to avoid potential problems with pin control mode (see Section 11.3).
	digitalWrite( m_pin_Clock, HIGH );
	digitalWrite( m_pin_SI, LOW );

	// Strobe CSn low / high.
	digitalWrite( m_pin_CS, LOW );
	digitalWrite( m_pin_CS, HIGH );

	// Hold CSn low and then high for at least 40 µs relative to pulling CSn low
	delayMicroseconds( 40 );

	// Pull CSn low and wait for SO to go low (CHIP_RDYn).
	digitalWrite( m_pin_CS, LOW );
	while ( digitalRead( m_pin_SO ) );

	// Issue the SRES strobe on the SI line.
	SendCommandStrobe( SRES );

	// When SO goes low again, reset is complete and the chip is in the IDLE state.
	while ( digitalRead( m_pin_SO ) );

	// Release the line
	digitalWrite( m_pin_CS, HIGH );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//

// NOTES: This function expects CS and SO to be LOW
void	CC1101::SPIW( byte _value ) {
	shiftOut( m_pin_SO, m_pin_Clock, MSBFIRST, _value);
}
byte	CC1101::SPIR() {
	byte	returnValue = shiftIn( m_pin_SI, m_pin_Clock, MSBFIRST );
	return returnValue;
}

byte	CC1101::SPIReadSingle( byte _address ) {
	byte	data;
	SPIRead( _address, 0x00, 1, &data );
	return data;
}
void	CC1101::SPIReadBurst( byte _address, uint32_t _dataLength, byte* _data ) {
	SPIRead( _address, 0x40U, _dataLength, _data );
}
void	CC1101::SPIRead( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data ) {
	digitalWrite( m_pin_CS, LOW );		// Enable the line
	while ( digitalRead( m_pin_SO ) );	// Ensure SO goes low
	delayns( 20 );						// 20ns before transmit (as per table 22)

	// Now shift out the opcode
	byte	opcode  = 0x80	// READ
					| _opcodeOR
					| (_address & 0x3F);
	SPIW( opcode );

	// Shift in data
	while ( _dataLength-- ) {
		// Wait between address/data and data/data bytes (as per table 22)
// 		delayns( 55 );	// Min 55ns before data in "Single Access"
// 		delayns( 76 );	// Min 76ns before data in "Burst Access"
		delayMicroseconds( 1 );

		*_data++ = SPIR();
	}

	delayns( 20 );						// 20ns after transmit (as per table 22)
	digitalWrite( m_pin_CS, HIGH );		// Release the line
}

void	CC1101::SPIWriteSingle( byte _address, byte _data ) {
	SPIWrite( _address, 0x00, 1, &_data );
}
void	CC1101::SPIWriteBurst( byte _address, uint32_t _dataLength, byte* _data ) {
	SPIWrite( _address, 0x40U, _dataLength, _data );
}
void	CC1101::SPIWrite( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data ) {
	digitalWrite( m_pin_CS, LOW );		// Enable the line
	while ( digitalRead( m_pin_SO ) );	// Ensure SO goes low
	delayns( 20 );						// 20ns before transmit (as per table 22)

	// Now shift out the opcode
	byte	opcode  = _opcodeOR
					| (_address & 0x3F);
	SPIW( opcode );

	// Shift out data
	while ( _dataLength-- ) {
		// Wait between address/data and data/data bytes (as per table 22)
// 		delayns( 55 );	// Min 55ns before data in "Single Access"
// 		delayns( 76 );	// Min 76ns before data in "Burst Access"
		delayMicroseconds( 1 );

		SPIW( *_data++ );
	}

	delayns( 20 );						// 20ns after transmit (as per table 22)
	digitalWrite( m_pin_CS, HIGH );		// Release the line
}

void	CC1101::SetPATable( byte _powerTable[8] ) {
	SPIWriteBurst( 0x3E, 8, _powerTable );
}

void	CC1101::SetFrequencyDeviation( float _deviationKHz ) {
	float	factor = (_deviationKHz * 0.001f / m_Fosc_MHz) * 131072.0f;
	byte	exponent = floor( log2( factor ) ) - 3;
			factor /= 1 << exponent;
			factor -= 8;
	byte	mantissa = ceil( factor );
	byte	value = ((exponent & 0x7) << 4) | (mantissa & 0x7);
	SetRegister( DEVIATN, value );
}
