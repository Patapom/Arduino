#include "Arduino.h"
#include "CC1101.h"

#include "Pom/Pom.h"

using namespace Pom;

#pragma region Enums

enum REGISTERS {
	IOCFG2	  =	0x00,
	IOCFG1	  =	0x01,
	IOCFG0	  =	0x02,
	FIFOTHR	  =	0x03,
	SYNC1	  =	0x04,
	SYNC0	  =	0x05,
	PKTLEN	  =	0x06,
	PKTCTRL1  =	0x07,
	PKTCTRL0  =	0x08,
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
	SRES			= 0x30,	// Reset chip.
	SFSTXON			= 0x31,	// Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX (with CCA): Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
	SXOFF			= 0x32,	// Turn off crystal oscillator.
	SCAL			= 0x33,	// Calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without setting manual calibration mode (MCSM0.FS_AUTOCAL=0)
	SRX				= 0x34,	// Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1.
	STX				= 0x35,	// In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled: Only go to TX if channel is clear.
	SIDLE			= 0x36,	// Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable.
	SWOR			= 0x38,	// Start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if WORCTRL.RC_PD=0.
	SPWD			= 0x39,	// Enter power down mode when CSn goes high.
	SFRX			= 0x3A,	// Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states.
	SFTX			= 0x3B,	// Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states.
	SWORRST			= 0x3C,	// Reset real time clock to Event1 value.
	SNOP			= 0x3D,	// No operation. May be used to get access to the chip status byte.
};

enum REGISTERS_STATUS {
	PARTNUM			= 0x30, // Part number for CC1101
	VERSION			= 0x31, // Current version number
	FREQEST			= 0x32, // Frequency Offset Estimate
	LQI				= 0x33, // Demodulator estimate for Link Quality
	RSSI			= 0x34, // Received signal strength indication
	MARCSTATE		= 0x35, // Control state machine state
	WORTIME1		= 0x36, // High byte of WOR timer
	WORTIME0		= 0x37, // Low byte of WOR timer
	PKTSTATUS		= 0x38, // Current GDOx status and packet status
	VCO_VC_DAC		= 0x39, // Current setting from PLL calibration module
	TXBYTES			= 0x3A, // Underflow and number of bytes in the TX FIFO
	RXBYTES			= 0x3B, // Overflow and number of bytes in the RX FIFO
	RCCTRL1_STATUS	= 0x3C, // Last RC oscillator calibration result
	RCCTRL0_STATUS	= 0x3D, // Last RC oscillator calibration result
};

#pragma endregion

void	delayns( uint32_t _nanoseconds ) {
	uint32_t	microseconds = (999 + _nanoseconds) / 1000;	// We don't have enough resolution for nanoseconds here so we round up to the next microsecond
	delayMicroseconds( microseconds );
}

const float	CC1101::DEFAULT_CARRIER_FREQUENCY_MHz = 800.0f;

CC1101::CC1101( byte _CS, byte _CLOCK, byte _SI, byte _SO, byte _GDO0, byte _GDO2 ) {
	// Setup pins
	m_pin_CS = _CS;
	m_pin_Clock = _CLOCK;
	m_pin_SI = _SI;
	m_pin_SO = _SO;
// 	m_pin_GDO0 = _GDO0;
// 	m_pin_GDO2 = _GDO2;

	pinMode( m_pin_Clock, OUTPUT );
	pinMode( m_pin_CS, OUTPUT );
	pinMode( m_pin_SO, INPUT );		// Master In Slave Out
	pinMode( m_pin_SI, OUTPUT );	// Master Out Slave In
 	pinMode( _GDO0, INPUT );
 	pinMode( _GDO2, INPUT );

	digitalWrite( m_pin_CS, HIGH );	// Clear the line

	// Setup SPI control register (cf. http://avrbeginners.net/architecture/spi/spi.html#spi_regs)
	{
		SPCR = 0x00;
		SPCR = (0 * _BV(SPIE))	// Disable SPI interrupt
			 | (1 * _BV(SPE))	// SPI Enable
			 | (1 * _BV(MSTR))	// Master
			 | (0 * _BV(DORD))	// Data Order 0 = MSB
			 | (0 * _BV(CPOL))	// Clock Polarity = HIGH (transfer on high)
			 | (0 * _BV(CPHA))	// Clock Phase = LEADING (transfer on rising edge)
			 | (0 * _BV(SPR1))	//
			 | (0 * _BV(SPR0));	// Clock Rate Select = 00 (Fosc/4) 

		// Read status and data
		byte	tmp = SPSR;
				tmp = SPDR;
				tmp++;
	}

	// Manually reset device
	Reset();
}

void	CC1101::Reset() {
//Serial.println( "Reset" );

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

//Serial.println( "CS LOW" );

	while ( digitalRead( m_pin_SO ) );

//Serial.println( "SO LOW" );

	// Issue the SRES strobe on the SI line.
//Serial.println( "Strobe" );
	SendCommandStrobe( SRES );
//DisplayStatus( SendCommandStrobe( SRES ) );
//Serial.println( "Strobe DONE!" );

	// When SO goes low again, reset is complete and the chip is in the IDLE state.
	while ( digitalRead( m_pin_SO ) );

//Serial.println( "SO LOW" );

	// Release the line
	digitalWrite( m_pin_CS, HIGH );

	// Synchronize our internal state
	ReadPKTCTRL0();
	ReadPKTCTRL1();

	// Perform custom reset operations
	InternalCustomReset();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// High-Level User Access Functions
void	CC1101::SetAddress( byte _address ) {
	SetRegister( ADDR, _address );
}

void	CC1101::SetChannel( byte _channel ) {
	SetRegister( CHANNR, _channel );
}

void	CC1101::EnableWhitening( bool _enable ) {
	m_enableWhitening = _enable;
	WritePKTCTRL0();
}
void	CC1101::UseFIFO( bool _useFIFO ) {
	m_useFIFO = _useFIFO;
	WritePKTCTRL0();
}
void	CC1101::EnableCRC( bool _enableCRC ) {
	m_enableCRC = _enableCRC;
	WritePKTCTRL0();
}
void	CC1101::SetPacketLengthConfig( PACKET_LENGTH_CONFIG _value ) {
	m_packetLengthConfig = _value;
	WritePKTCTRL0();
}
void	CC1101::EnablePacketAddressCheck( bool _enable ) {
	m_enablePacketAddressCheck = _enable;
	WritePKTCTRL1();
}
void	CC1101::SetPacketLength( byte _length ) {
	SetRegister( PKTLEN, _length );
}
void	CC1101::SetSyncWord( uint16_t _syncWord ) {
	SetRegister( SYNC1, _syncWord >> 8 );
	SetRegister( SYNC0, _syncWord & 0xFF );
}

void	CC1101::SetCarrierFrequency( float _Fcarrier_MHz ) {
	// Fcarrier = Fosc * FREQ * 2^-16
	float		fFREQ = _Fcarrier_MHz * 65536.0f / Fosc_MHz;
	uint32_t	FREQ = floor( fFREQ );
				FREQ = min( FREQ, 0x3FFFFF );
	byte	frequency[3];
	frequency[0] = (FREQ >> 16) & 0xFF;
	frequency[1] = (FREQ >> 8) & 0xFF;
	frequency[2] = FREQ & 0xFF;
	SPIWriteBurst( FREQ2, 3, frequency );
}

void	CC1101::SetIntermediateFrequency( float _F_KHz ) {
	// 	IF = Fosc * FREQ_IF * 2^-10
	uint32_t	FREQ_IF = floor( _F_KHz * 0.001f * 1024.0f / Fosc_MHz );
				FREQ_IF = min( FREQ_IF, 0xF );
	SetRegister( FSCTRL1, FREQ_IF );
}

void	CC1101::SetFrequencyOffset( float _Foffset_KHz ) {
	// Frequency offset (FREQOFF) added to the base frequency before being used by the frequency synthesizer.
	// Resolution is FXTAL/2^14 (1.59kHz-1.65kHz); range is ±202 kHz to ±210 kHz, dependent of XTAL frequency
	const float	stepSize = Fosc_MHz / 16384.0f;		// Step resolution
	int		stepIndex = _Foffset_KHz / stepSize;
	byte	value = clamp( stepIndex, -128, 127 );
	SetRegister( FSCTRL0, value );
}

void	CC1101::SetChannelBandwithAndDataRate( float _bandwidth_KHz, float _dataRate_KBauds ) {
	// Channel Bandwidth = Fosc / (8*(4+CHANBW_M)*2^CHANBW_E)
	float	valueBW = Fosc_MHz / (0.001f * _bandwidth_KHz);
	byte	expBW = byte( max( 5, floor( log2( valueBW ) ) ) );
			valueBW /= 1 << expBW;
			expBW = min( 3, expBW - 5 );
	byte	mantissaBW = byte( clamp( floor( 4 * (valueBW - 1.0f) ), 0, 3 ) );

	// Data Rate = Fosc * (256 + DRATE_M)*2^(DRATE_E-28)
	float	valueDR = 0.001f * _dataRate_KBauds / Fosc_MHz;
	int		expDR = floor( log2( valueDR ) );
			valueDR *= 1 << (-expDR);
	byte	mantissaDR = byte( clamp( floor( 256 * (valueDR - 1.0f) ), 0, 255 ) );

	// Send values
	byte	value4  = (expBW << 6)
					| (mantissaBW << 4)
					| clamp( 20+expDR, 0, 15 );
	SetRegister( MDMCFG4, value4 );
	SetRegister( MDMCFG3, mantissaDR );
}

void	CC1101::SetChannelSpacing( float _spacing_KHz ) {
	// Channel spacing frequency = Fosc * (256+CHANSPC_M)*2^(CHANSPC_E-18)
	float	value = 0.001f * _spacing_KHz / Fosc_MHz;
	int		exponent = floor( log2( value ) );
			value *= 1 << (-exponent);
	byte	mantissa = byte( clamp( floor( 256 * (value - 1.0f) ), 0, 255 ) );

	// Send values
	byte	value1  = (0 * _BV(7))		// No FEC encoding
					| (2 << 4)			// Use 4 bytes in preamble
					| clamp( 10+exponent, 0, 3 );
	SetRegister( MDMCFG1, value1 );
	SetRegister( MDMCFG0, mantissa );
}

void	CC1101::SetFrequencyDeviation( float _deviation_KHz ) {
	// 	Nominal frequency deviation from the carrier Fdev = Fosc * (8+DEVIATION_M)^2(DEVIATION_E-17)
	float	value = 0.001f *_deviation_KHz / Fosc_MHz;
	int		exponent = floor( log2( value ) );
			value *= 1 << (-exponent);
	byte	mantissa = byte( clamp( floor( 8 * (value - 1.0f) ), 0, 7 ) );

	// Send value
	byte	dev = (clamp( 14+exponent, 0, 7 ) << 4)
				| (mantissa & 0x7);
	SetRegister( DEVIATN, dev );
}

CC1101::MACHINE_STATE	CC1101::ReadFSMState() {
	byte	value = ReadStatusRegister( MARCSTATE );
	return MACHINE_STATE( value & 0x1F );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Med-Level Helpers Functions

byte	CC1101::ReadPATable( byte _powerTable[8] ) {
	return SPIReadBurst( 0x3E, 8, _powerTable );
}
byte	CC1101::WritePATable( byte _powerTable[8] ) {
	return SPIWriteBurst( 0x3E, 8, _powerTable );
}

byte	CC1101::SetRegister( byte _address, byte _value ) {
	return SPIWriteSingle( _address, _value );
}
byte	CC1101::SendCommandStrobe( byte _command ) {
	return SPITransfer( _command );
}
byte	CC1101::ReadStatus() {
	return SPITransfer( SNOP );
}

byte	CC1101::ReadStatusRegister( byte _address ) {
	byte	value;
	SPIReadBurst( _address, 1, &value );
	return value;
}

void	CC1101::WritePKTCTRL0() {
	byte	value = (m_enableWhitening ? 0x40 : 0x00)
				  | (m_useFIFO ? 0x00 : 0x10)	// Synchronous mode otherwise
				  | (m_enableCRC ? 0x04 : 0x00)
				  | (m_packetLengthConfig & 0x3);
	SetRegister( PKTCTRL0, value );
}
void	CC1101::ReadPKTCTRL0() {
	byte	value = SPIReadSingle( PKTCTRL0 );
	m_enableWhitening = (value & 0x40) != 0;
	m_useFIFO = (value & 0x30) == 0;
	m_enableCRC = (value & 0x04) != 0;
	m_packetLengthConfig = PACKET_LENGTH_CONFIG( value & 0x3 );
}

void	CC1101::WritePKTCTRL1() {
	byte	value = (m_enablePacketAddressCheck ? 0x01 : 0x00)
				  | 0x04;	// No preamble quality estimator, disable auto flush on CRC error, append 2 status bytes at the end of packet payloads
	SetRegister( PKTCTRL1, value );
}
void	CC1101::ReadPKTCTRL1() {
	byte	value = SPIReadSingle( PKTCTRL1 );
	m_enablePacketAddressCheck = (value & 0x03) != 0;
}

void	CC1101::InternalCustomReset() {
	// Change GDO modes to avoid costly CLK_XOSC settings
	#ifdef SPI_DEBUG_VERBOSE
		Serial.println( "Set GPOx modes to CHIP_RDYn" );
	#endif
	SetRegister( IOCFG0, 0x29 );	// Signal CHIP_RDYn
	SetRegister( IOCFG2, 0x29 );	// Signal CHIP_RDYn

	// Setup custom PA table (even though only the first byte is used because FREND0.PA_POWER is set to 0)
	byte	temp[8] = { 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6 };
	WritePATable( temp );

	#ifdef SPI_DEBUG_VERBOSE
		Serial.println( "<PATable>" );
		ReadPATable( temp );
		for ( int i=0; i < 8; i++ )
			Serial.println( temp[i], HEX );
		Serial.println( "</PATable>" );
	#endif

	// Setup some default registers
	// Default values after a RESET are:
	// 	IOCFG2			(0x00) = 0x29	Signal CHIP_RDYn on GDO2, No invert
	// 	IOCFG1			(0x01) = 0x2E	3-state on GDO1 (default to be able to use SO), No invert
	// 	IOCFG0			(0x02) = 0x3F	Signal CLK_XOSC/192, No invert
	// 	FIFOTHR			(0x03) = 0x07	RX Attenuation = 0dB. Set the threshold for the TX FIFO and RX FIFO to 7 = 33 (half the queue size)
	// 	SYNC1			(0x04) = 0xD3	Sync Word MSB
	// 	SYNC0			(0x05) = 0x91	Sync Word LSB
	// 	PKTLEN			(0x06) = 0xFF	Default Packet Length
	// 	PKTCTRL1		(0x07) = 0x04	Always accept sync word. Don't autoflush when CRC is wrong. Append 2 status bytes to packets. No address check.
	// 	PKTCTRL0		(0x08) = 0x45	Data whitening enabled. Packet format normal mode (use FIFO). CRC enabled. Variable packet length.
	// 	ADDR			(0x09) = 0x00	Broadcast address 0x00
	// 	CHANNR			(0x0A) = 0x00	Broadcast on channel 0x00
	// 	FSCTRL1			(0x0B) = 0x0F	FREQ_IF = 15 (IF = Fosc * FREQ_IF * 2^-10 = 381KHz) 
	// 	FSCTRL0			(0x0C) = 0x00	Frequency offset (FREQOFF) added to the base frequency before being used by the frequency synthesizer.
	// 	FREQ2			(0x0D) = 0x1E	MSB of Fcarrier = Fosc * FREQ * 2^-16 = 26Mhz * 0x1EC4EC / 65536 = 26 * 30.76922607421875 = 799.9998779296875 MHz
	// 	FREQ1			(0x0E) = 0xC4	MidSB of Fcarrier
	// 	FREQ0			(0x0F) = 0xEC	LSB of Fcarrier
	// 	MDMCFG4			(0x10) = 0x8C	Channel Bandwidth = Fosc / (8*(4+CHANBW_M)*2^CHANBW_E) = 203.125 KHz (with CHANBW_M=0 & CHANBW_E=2). DRATE_E = 12
	// 	MDMCFG3			(0x11) = 0x22	Data Rate = Fosc * (256 + DRATE_M)*2^(DRATE_E-28) = 26 * 0.004425048828125 = 115.05126953125 KBauds (with DRATE_M=34 & DRATE_E=12)
	// 	MDMCFG2			(0x12) = 0x02	Enable digital DC blocking (better sensitivity). Modulation format = 2-FSK. Disable Manchester Encoding. 16/16 Sync word bits must match.
	// 	MDMCFG1			(0x13) = 0x22	Forward Error Correction (FEC) disabled. A minimum of 4 preamble bytes must be transmitted. CHANSPC_E = 2
	// 	MDMCFG0			(0x14) = 0xF8	Channel spacing frequency = Fosc * (256+CHANSPC_M)*2^(CHANSPC_E-18) = 26 * 0.0076904296875 = 199.951171875 KHz (with CHANSPC_M = 0xF8 and CHANSPC_E = 2)
	// 	DEVIATN			(0x15) = 0x47	Nominal frequency deviation from the carrier Fdev = Fosc * (8+DEVIATION_M)^2(DEVIATION_E-17) = 26 * 0.0018310546875 = 47.607421875 KHz (with DEVIATION_E = 4 & DEVIATION_M = 7)
	// 	MCSM2			(0x16) = 0x07	No direct RX termination based on RSSI measurement. Only check Sync word on RX_TIME expired. Wait until end of packet for timeout. 
	// 	MCSM1			(0x17) = 0x30	Clear Channel Assessment (CCA) Mode = If RSSI below threshold unless currently receiving a packet. RXOFF_MODE goes to IDLE after packet received. TXOFF_MODE goes to IDLE after packet received.
	// 	MCSM0			(0x18) = 0x04	Auto calibration disabled. Xosc stabilized after counter reaches 16. Disable pin radio control option. Force Xosc to stay on during sleep state is DISABLED.
	// 	FOCCFG			(0x19) = 0x76	Freeze demodulator state until CS goes high. Frequency compensation loop gain BEFORE sync word detection to 3K. Fres. comp AFTER sync word to K/2. Saturation point for freq. offset compensation algorithm = +-BWchan/4
	// 	BSCFG			(0x1A) = 0x6C	Clock recovery feedback loop integral gain BEFORE sync word to 2Ki. Proportional gain to 3Kp. Ki/2 for AFTER sync word integral gain. Kp for AFTER sync word proportional gain. Saturation point for data rate offset compensation algorithm = 0 (no compensation performed)
	// 	AGCTRL2			(0x1B) = 0x03	All gain settings can be used. Maximum possible LNA + LNA 2 gain. Average amplitude for from digital channel filter = 33 dB.
	// 	AGCTRL1			(0x1C) = 0x40	LNA gain is decreased first. Relative carrier sense threshold disabled. Carrier sense RSSI threshold at MAGN_TARGET.
	// 	AGCTRL0			(0x1D) = 0x91	Medium hysteresis on magnitude deviation. 16 channel filter samples. Never freeze AGC. Use 16 samples for amplitude filtering.
	// 	WOREVT1			(0x1E) = 0x87	High byte of EVENT0 timeout register t_EVENT0 = 750 / F_osc * EVENT0*2^(5*WOR_RES) = 750 / 26.0E6 * 0x876B * 2^(5*0) ~= 1s
	// 	WOREVT0			(0x1F) = 0x6B	High byte of EVENT0 timeout register
	// 	WORCTRL			(0x20) = 0xF8	Power down signal not set. Timeout of EVENT1 = 48 clocks. RC oscillator calibration enabled. WOR_RES = 0
	// 	FREND1			(0x21) = 0x56	<ADVANCED> Front-end RX current settings (not very detailed :'().
	// 	FREND0			(0x22) = 0x10	<ADVANCED> Front-end TX settings (not very detailed either). PA power setting set to index 0 in PATABLE.
	// 	FSCAL3			(0x23) = 0xA9	<ADVANCED> Frequency synthesizer calibration settings (not very detailed).
	// 	FSCAL2			(0x24) = 0x0A	<ADVANCED> Basically, all these settings are set by the Texas Instrument's proprietary tool SmartRF Studio (http://www.ti.com/tool/SMARTRFTM-STUDIO)
	// 	FSCAL1			(0x25) = 0x20	<ADVANCED> 
	// 	FSCAL0			(0x26) = 0x0D	<ADVANCED> 
	// 	RCCTRL1			(0x27) = 0x41	<ADVANCED> 
	// 	RCCTRL0			(0x28) = 0x00	<ADVANCED> 
	// 	FSTEST			(0x29) = 0x59	<TEST ONLY> Don't write!
	// 	PTEST			(0x2A) = 0x7F	Writing 0xBF to this register makes the on-chip temperature sensor available in the IDLE state. The default 0x7F value should then be written back before leaving the IDLE state
	// 	AGCTEST			(0x2B) = 0x3F	<TEST ONLY> Don't write!
	// 	TEST2			(0x2C) = 0x88	<TEST ONLY>
	// 	TEST1			(0x2D) = 0x31	<TEST ONLY>
	// 	TEST0			(0x2E) = 0x0B	<TEST ONLY>
	//
	// 	<<< 0x2F is undefined >>>
	//
	// Status registers are:
	// 	PARTNUM			(0x30) = 0x00
	// 	VERSION			(0x31) = 0x14
	// 	FREQEST			(0x32) = 0x00
	// 	LQI				(0x33) = 0x05
	// 	RSSI			(0x34) = 0x80
	// 	MARCSTATE		(0x35) = 0x01
	// 	WORTIME1		(0x36) = 0x00
	// 	WORTIME0		(0x37) = 0x00
	// 	PKTSTATUS		(0x38) = 0x00
	// 	VCO_VC_DAC		(0x39) = 0x94
	// 	TXBYTES			(0x3A) = 0x00
	// 	RXBYTES			(0x3B) = 0x00
	// 	RCCTRL1_STATUS	(0x3C) = 0x00
	// 	RCCTRL0_STATUS	(0x3D) = 0x00

// This is the configuration proposed by the ELECHOUSE library
// 	SpiWriteReg(FSCTRL1,  0x08);	// IF = 203KHz
// 	SpiWriteReg(FREQ2,    0x10);	// 433MHz
// 	SpiWriteReg(FREQ1,    0xA7);
// 	SpiWriteReg(FREQ0,    0x62);
// 	SpiWriteReg(IOCFG2,   0x0B); 	//serial clock.synchronous to the data in synchronous serial mode
// 	SpiWriteReg(IOCFG0,   0x06);  	//asserts when sync word has been sent/received, and de-asserts at the end of the packet 
// 	SpiWriteReg(PKTLEN,   0x3D); 	//61 bytes max length

// 	SpiWriteReg(MDMCFG4,  0x5B);
// 	SpiWriteReg(MDMCFG3,  0xF8);
// 	SpiWriteReg(MDMCFG2,  0x03);
// 	SpiWriteReg(MDMCFG1,  0x22);
// 	SpiWriteReg(MDMCFG0,  0xF8);
// 	SpiWriteReg(FREND1,   0xB6);
// 	SpiWriteReg(MCSM0 ,   0x18);
// 	SpiWriteReg(FOCCFG,   0x1D);
// 	SpiWriteReg(BSCFG,    0x1C);
// 	SpiWriteReg(AGCTRL2,  0xC7);
// 	SpiWriteReg(AGCTRL1,  0x00);
// 	SpiWriteReg(AGCTRL0,  0xB2);
// 	SpiWriteReg(FSCAL3,   0xEA);
// 	SpiWriteReg(FSCAL2,   0x2A);
// 	SpiWriteReg(FSCAL1,   0x00);
// 	SpiWriteReg(FSCAL0,   0x11);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Low-Level R/W Functions

// NOTES: This function expects CS and SO to be LOW
// Details can be found at http://avrbeginners.net/architecture/spi/spi.html and https://www.arduino.cc/en/Tutorial/SPIEEPROM
//
byte	CC1101::SPITransfer( byte _value ) {
	#ifdef SPI_DEBUG_VERBOSE
		Serial.print( "SPI Write 0x" );
		Serial.print( _value, HEX );
		#if SPI_DEBUG_VERBOSE == 2
			Serial.print( " [" );
			DisplayDecodedWrittenValue( _value );
			Serial.print( "]" );
		#endif
	#endif

	SPDR = _value;						// Write value to be transfered

	// (Stolen from SPI library)
	// The following NOP introduces a small delay that can prevent the wait loop form iterating when running at the maximum speed.
	// This gives about 10% more speed, even if it seems counter-intuitive. At lower speeds it is unnoticed.
	asm volatile( "nop" );

	while ( !(SPSR & (1 << SPIF)) );	// Wait until shifting is complete

	_value = SPDR;						// Read received value

	#ifdef SPI_DEBUG_VERBOSE
		Serial.print( " - Read 0x" );
		Serial.println( _value, HEX );
	#endif

	return _value;
}

byte	CC1101::SPIReadSingle( byte _address ) {
	byte	data;
	SPIRead( _address, 0x00, 1, &data );	// Status is simply ignored
	return data;
}
byte	CC1101::SPIReadBurst( byte _address, uint32_t _dataLength, byte* _data ) {
	byte	status = SPIRead( _address, 0x40U, _dataLength, _data );
	#ifdef SPI_DEBUG_VERBOSE
		Serial.println( "BURST END" );
	#endif
	return status;
}
byte	CC1101::SPIRead( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data ) {
	digitalWrite( m_pin_CS, LOW );		// Enable the line
	while ( digitalRead( m_pin_SO ) );	// Ensure SO goes low
	delayns( 20 );						// 20ns before transmit (as per table 22)

	// Now shift out the opcode
	byte	opcode  = 0x80	// READ
					| _opcodeOR
					| (_address & 0x3F);
	byte	status = SPITransfer( opcode );

	// Shift in data
	while ( _dataLength-- ) {
		// Wait between address/data and data/data bytes (as per table 22)
// 		delayns( 55 );	// Min 55ns before data in "Single Access"
// 		delayns( 76 );	// Min 76ns before data in "Burst Access"
		delayMicroseconds( 1 );

		*_data++ = SPITransfer( SNOP );
	}

	delayns( 20 );						// 20ns after transmit (as per table 22)
	digitalWrite( m_pin_CS, HIGH );		// Release the line

	return status;
}

byte	CC1101::SPIWriteSingle( byte _address, byte _data ) {
	return SPIWrite( _address, 0x00, 1, &_data );
}
byte	CC1101::SPIWriteBurst( byte _address, uint32_t _dataLength, byte* _data ) {
	byte	status = SPIWrite( _address, 0x40U, _dataLength, _data );
	#ifdef SPI_DEBUG_VERBOSE
		Serial.println( "BURST END" );
	#endif
	return status;
}
byte	CC1101::SPIWrite( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data ) {
	digitalWrite( m_pin_CS, LOW );		// Enable the line
	while ( digitalRead( m_pin_SO ) );	// Ensure SO goes low
	delayns( 20 );						// 20ns before transmit (as per table 22)

	// Now shift out the opcode
	byte	opcode  = _opcodeOR
					| (_address & 0x3F);
	byte	status = SPITransfer( opcode );

	// Shift out data
	while ( _dataLength-- ) {
		// Wait between address/data and data/data bytes (as per table 22)
// 		delayns( 55 );	// Min 55ns before data in "Single Access"
// 		delayns( 76 );	// Min 76ns before data in "Burst Access"
		delayMicroseconds( 1 );

		SPITransfer( *_data++ );
	}

	delayns( 20 );						// 20ns after transmit (as per table 22)
	digitalWrite( m_pin_CS, HIGH );		// Release the line

	return status;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// DEBUG
#ifdef SPI_DEBUG_VERBOSE

void	CC1101::DisplayStatus( byte _status ) {
	Serial.print( (_status & 0x80) ? "Chip NOT READY " : "Chip READY " );
	switch ( (_status >> 4) & 0x7 ) {
		case 0:	Serial.print( "IDLE" ); break;
		case 1:	Serial.print( "RX" ); break;
		case 2:	Serial.print( "TX" ); break;
		case 3:	Serial.print( "FSTXON" ); break;
		case 4:	Serial.print( "CALIBRATE" ); break;
		case 5:	Serial.print( "SETTLING" ); break;
		case 6:	Serial.print( "RXFIFO_OVERFLOW" ); break;
		case 7:	Serial.print( "TXFIFO_UNDERFLOW" ); break;
	}
	Serial.print( " Av. Bytes: " );
	Serial.println( _status & 0x0F );
}

void	CC1101::DisplayDecodedWrittenValue( byte _writtenValue ) {
	Serial.print( (_writtenValue & 0x80) != 0 ? "READ " : "WRITE " );
	Serial.print( (_writtenValue & 0x40) != 0 ? "BURST " : "SINGLE " );

	byte	address = _writtenValue & 0x3F;
	if ( address < 0x30 ) {
		// Register access
		Serial.print( "REG " );
		switch ( address ) {
			case IOCFG2		:	Serial.print( "IOCFG2" ); break;
			case IOCFG1		:	Serial.print( "IOCFG1" ); break;
			case IOCFG0		:	Serial.print( "IOCFG0" ); break;
			case FIFOTHR	:	Serial.print( "FIFOTHR" ); break;
			case SYNC1		:	Serial.print( "SYNC1" ); break;
			case SYNC0		:	Serial.print( "SYNC0" ); break;
			case PKTLEN		:	Serial.print( "PKTLEN" ); break;
			case PKTCTRL0	:	Serial.print( "PKTCTRL0" ); break;
			case PKTCTRL1	:	Serial.print( "PKTCTRL1" ); break;
			case ADDR		:	Serial.print( "ADDR" ); break;
			case CHANNR		:	Serial.print( "CHANNR" ); break;
			case FSCTRL1	:	Serial.print( "FSCTRL1" ); break;
			case FSCTRL0	:	Serial.print( "FSCTRL0" ); break;
			case FREQ2		:	Serial.print( "FREQ2" ); break;
			case FREQ1		:	Serial.print( "FREQ1" ); break;
			case FREQ0		:	Serial.print( "FREQ0" ); break;
			case MDMCFG4	:	Serial.print( "MDMCFG4" ); break;
			case MDMCFG3	:	Serial.print( "MDMCFG3" ); break;
			case MDMCFG2	:	Serial.print( "MDMCFG2" ); break;
			case MDMCFG1	:	Serial.print( "MDMCFG1" ); break;
			case MDMCFG0	:	Serial.print( "MDMCFG0" ); break;
			case DEVIATN	:	Serial.print( "DEVIATN" ); break;
			case MCSM2		:	Serial.print( "MCSM2" ); break;
			case MCSM1		:	Serial.print( "MCSM1" ); break;
			case MCSM0		:	Serial.print( "MCSM0" ); break;
			case FOCCFG		:	Serial.print( "FOCCFG" ); break;
			case BSCFG		:	Serial.print( "BSCFG" ); break;
			case AGCTRL2	:	Serial.print( "AGCTRL2" ); break;
			case AGCTRL1	:	Serial.print( "AGCTRL1" ); break;
			case AGCTRL0	:	Serial.print( "AGCTRL0" ); break;
			case WOREVT1	:	Serial.print( "WOREVT1" ); break;
			case WOREVT0	:	Serial.print( "WOREVT0" ); break;
			case WORCTRL	:	Serial.print( "WORCTRL" ); break;
			case FREND1		:	Serial.print( "FREND1" ); break;
			case FREND0		:	Serial.print( "FREND0" ); break;
			case FSCAL3		:	Serial.print( "FSCAL3" ); break;
			case FSCAL2		:	Serial.print( "FSCAL2" ); break;
			case FSCAL1		:	Serial.print( "FSCAL1" ); break;
			case FSCAL0		:	Serial.print( "FSCAL0" ); break;
			case RCCTRL1	:	Serial.print( "RCCTRL1" ); break;
			case RCCTRL0	:	Serial.print( "RCCTRL0" ); break;
			case FSTEST		:	Serial.print( "FSTEST" ); break;
			case PTEST		:	Serial.print( "PTEST" ); break;
			case AGCTEST	:	Serial.print( "AGCTEST" ); break;
			case TEST2		:	Serial.print( "TEST2" ); break;
			case TEST1		:	Serial.print( "TEST1" ); break;
			case TEST0		:	Serial.print( "TEST0" ); break;
		}
	} else if ( (_writtenValue & 0xC0) == 0xC0 ) {
		// Reading status register
		Serial.print( "STATUS " );
		switch ( address ) {
			case PARTNUM		:	Serial.print( "PARTNUM" ); break;
			case VERSION		:	Serial.print( "VERSION" ); break;
			case FREQEST		:	Serial.print( "FREQEST" ); break;
			case LQI			:	Serial.print( "LQI" ); break;
			case RSSI			:	Serial.print( "RSSI" ); break;
			case MARCSTATE		:	Serial.print( "MARCSTATE" ); break;
			case WORTIME1		:	Serial.print( "WORTIME1" ); break;
			case WORTIME0		:	Serial.print( "WORTIME0" ); break;
			case PKTSTATUS		:	Serial.print( "PKTSTATUS" ); break;
			case VCO_VC_DAC		:	Serial.print( "VCO_VC_DAC" ); break;
			case TXBYTES		:	Serial.print( "TXBYTES" ); break;
			case RXBYTES		:	Serial.print( "RXBYTES" ); break;
			case RCCTRL1_STATUS	:	Serial.print( "RCCTRL1_STATUS" ); break;
			case RCCTRL0_STATUS	:	Serial.print( "RCCTRL0_STATUS" ); break;
		}
	} else {
		// Issue strobe command
		Serial.print( "STROBE " );
		switch ( address ) {
			case SRES	: Serial.print( "SRES" ); break;
			case SFSTXON: Serial.print( "SFSTXON" ); break;
			case SXOFF	: Serial.print( "SXOFF" ); break;
			case SCAL	: Serial.print( "SCAL" ); break;
			case SRX	: Serial.print( "SRX" ); break;
			case STX	: Serial.print( "STX" ); break;
			case SIDLE	: Serial.print( "SIDLE" ); break;
			case SWOR	: Serial.print( "SWOR" ); break;
			case SPWD	: Serial.print( "SPWD" ); break;
			case SFRX	: Serial.print( "SFRX" ); break;
			case SFTX	: Serial.print( "SFTX" ); break;
			case SWORRST: Serial.print( "SWORRST" ); break;
			case SNOP	: Serial.print( "SNOP" ); break;
		}
	}
}

void	CC1101::DumpAllRegisters( byte _registerValues[0x3E] ) {
	SPIReadBurst( 0x00, 0x2F, _registerValues );	// Read initial register values

	_registerValues[0x2F] = 0x00;	// Undefined

	for ( int i=0; i < 0xE; i++ ) {
		_registerValues[PARTNUM+i] = ReadStatusRegister( PARTNUM + i );
	}
}

#endif