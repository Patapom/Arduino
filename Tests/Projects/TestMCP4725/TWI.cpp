#include "Pom/Pom.h"
#include "TWI.h"

#ifndef INSTALL_TWI_HANDLER
#define INSTALL_TWI_HANDLER	// Define this to install our own TWI interrupt handler
#endif

//////////////////////////////////////////////////////////////////////////
// The Two-Wire Interface (TWI) is described in section 22 of the Atmel documentation.
//
// The TWI interrupt is located at vector #24 (address 0x0030)
// 
// The TWI registers are:
//		Address 0xB8 => TWBR	(TWI Bit Rate Register, section 22.9.1)
//		Address 0xB9 => TWSR	(TWI Status Register, section 22.9.3)
//		Address 0xBA => TWAR	(TWI Address Register (i.e. the address of the MCU when used as a slave), section 22.9.5)
//		Address 0xBB => TWDR	(TWI Data Register, section 22.9.4)
//		Address 0xBC => TWCR	(TWI Control Register, section 22.9.2)
//		Address 0xBD => TWAMR	(TWI (Slave) Address Mask Register, section 22.9.6)
//
// Control Flags for the Control Register (TWCR)
//	Bit #	|	Name	|	Function
// ---------+-----------+-----------------
//	  0		|	TWIE	|	Interrupt Enable. When this bit is written to one, and the I-bit in SREG is set, the TWI interrupt request will be activated for as long as the TWINT Flag is high.
//	  1		|			|	<Reserved>
//	  2		|	TWEN	|	Global module enable. Enables TWI operation and activates the TWI interface.
//	  3		|	TWWC	|	Write Collision Flag. Set when attempting to write to the TWDR while TWINT is low.
//	  4		|	TWSTO	|	Stop Condition bit. Must be written to release master status.
//	  5		|	TWSTA	|	Start Condition bit. Must be written when the application wants master status on the TWI bus
//	  6		|	TWEA	|	Enable Acknowledge bit. Must be set in receive mode if a data byte has been received.
//	  7		|	TWINT	|	Interrupt Flag. This bit is set by hardware when the TWI has finished its current job and expects application software response
//
// Status codes from the Status Register (TWSR) (shifted by 3, since lowest 3 bits are not part of the code)
//
//	Code  |  Shifted>>3	|	Meaning
//	0x00  |		0x00	|	Bus error
//	0x08  |		0x01	|	A START condition has been transmitted
//	0x10  |		0x02	|	A repeated START condition has been transmitted
//	--- From table 22.2	(MATER TRANSMIT MODE)
//	0x18  |		0x03	|	SLA+W has been transmitted; ACK has been received
//	0x20  |		0x04	|	SLA+W has been transmitted; NOT ACK has been received	
//	0x28  |		0x05	|	Data byte has been transmitted; ACK has been returned
//	0x30  |		0x06	|	Data byte has been transmitted; NOT ACK has been returned
//	0x38  |		0x07	|	Arbitration lost in SLA+R/W or NOT ACK bit
//	--- From table 22.3	(MASTER RECEIVE MODE)
//	0x40  |		0x08	|	SLA+R has been transmitted; ACK has been received
//	0x48  |		0x09	|	SLA+R has been transmitted; NOT ACK has been received
//	0x50  |		0x0A	|	Data byte has been received; ACK has been returned
//	0x58  |		0x0B	|	Data byte has been received; NOT ACK has been returned

//	--- From table 22.4 (SLAVE TRANSMIT MODE)
//	0x60  |		0x0C	|	Own SLA+W has been received; ACK has been returned
//	0x68  |		0x0D	|	Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
//	0x70  |		0x0E	|	General call address has been received; ACK has been returned
//	0x78  |		0x0F	|	Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
//	0x80  |		0x10	|	Previously addressed with own SLA+W; data has been received; ACK has been returned
//	0x88  |		0x11	|	Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
//	0x90  |		0x12	|	Previously addressed with general call; data has been received; ACK has been returned
//	0x98  |		0x13	|	Previously addressed with general call; data has been received; NOT ACK has been returned
//	0xA0  |		0x14	|	A STOP condition or repeated START condition has been received while still addressed as Slave
//	--- From table 22.5 (SLAVE RECEIVE MODE)
//	0xA8  |		0x15	|	SLA+R has been received; ACK has been returned
//	0xB0  |		0x16	|	Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
//	0xB8  |		0x17	|	Data byte in TWDR has been transmitted; ACK has been received
//	0xC0  |		0x18	|	Data byte in TWDR has been transmitted; NOT ACK has been received
//	0xC8  |		0x19	|	Last data byte in TWDR has been transmitted (TWEA = “0”); ACK has been received
//	0xD0  |		0x1A	|
//	0xD8  |		0x1B	|
//	0xE0  |		0x1C	|
//	0xE8  |		0x1D	|
//	0xF0  |		0x1E	|
//	0xF8  |		0x1F	|	No relevant state
//
//////////////////////////////////////////////////////////////////////////
//
// Install the TWI interrupt handler
#ifdef INSTALL_TWI_HANDLER

static TWI*	gs_TWI = NULL;
ISR( TWI_vect ) {
	gs_TWI->InterruptHandler();
}

#endif

TWI::TWI()
	: m_bufferIndex( 0 )
	, m_status( STATUS_WAITING )
	, m_master( true )
	, m_transmit( true )
{
	#ifdef INSTALL_TWI_HANDLER
		gs_TWI = this;	// Set us as the only TWI instance
	#endif
	m_bufferIndexMask = 0xFF >> (8-MAX_LENGTH_POT);

	// Enable pull-up on data and clock lines
	analogWrite( SDA, HIGH );
	analogWrite( SCL, HIGH );

	// Enable twi module, acknowled bits emission, and twi interrupt
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
}

TWI::~TWI() {
	// Disable pull-up on data and clock lines
	analogWrite( SDA, LOW );
	analogWrite( SCL, LOW );
}

// According to section 22.5.2 of the Atmel doc, the serial clock frequency is computed as:
//	SCL frequency = CPU Frequency / (16 + 2 * TWBR * prescaler)
// Thus:
//	TWBR = (CPU Frequency / SCL frequency - 16) / (2 * prescaler)
//
void	TWI::SetFrequency( U32 _frequencyMHz, PRESCALER_VALUE _prescaler ) {

	U32	bitRate = F_CPU / _frequencyMHz - 16;
	switch ( _prescaler ) {
		case PRESCALE_1: bitRate >>= 1; break;
		case PRESCALE_4: bitRate >>= 3; break;
		case PRESCALE_16: bitRate >>= 5; break;
		case PRESCALE_64: bitRate >>= 7; break;
	}

	TWBR = bitRate;
	TWSR = (TWSR & 0x7C) | U8(_prescaler);
}

void	TWI::BeginTransmit( U8 _address ) {
	if ( _address == m_address && m_master && m_transmit )
		return;	// No change...

	cli();
	m_address = _address;
	m_master = true;
	m_transmit = true;
	m_needsRestart = m_status != STATUS_WAITING;	// If already busy and the address or mode changed, we need to restart
	sei();
}
// void	TWI::BeginReply( U8 _address ) {
// 	if ( _address == m_address && m_master && m_transmit )
// 		return;	// No change...
// 
// 	cli();
// 	m_address = _address;
// 	m_master = false;
// 	m_transmit = true;
// 	m_needsRestart = m_status != STATUS_WAITING;	// If already busy and the address or mode changed, we need to restart
// 	sei();
// }

void	TWI::Push( U8* _data, U8 _length ) {
	cli();	// Prevent any interruption

	m_dataLength += _length;	// We have stuff to transmit!
	for ( ; _length > 0; _length-- ) {
		m_ringBuffer[m_bufferIndex++] = *_data++;
		m_bufferIndex &= m_bufferIndexMask;
	}

	if ( m_status != STATUS_TRANSMITTING || m_needsRestart ) {
		m_needsRestart = false;
		TWCR = CTRL_ACK | _BV(TWSTA);	// Send a start bit, otherwise let the transmission continue...
	}

	sei();	// Continue...
}

void	TWI::BeginReceive( U8 _address ) {
	if ( _address == m_address && m_master && !m_transmit )
		return;	// No change...

	cli();
	m_address = _address;
	m_master = true;
	m_transmit = false;
	m_needsRestart = m_status != STATUS_WAITING;	// If already busy and the address or mode changed, we need to restart
	sei();
}
U8	TWI::GetAvailableDataLength() const {
	cli();
	U8	count = m_dataLength;
	sei();
	return count;
}
U8	TWI::Pull( U8* _data, U8 _length ) {
	cli();
	U8	count = m_dataLength;
	if ( _length != 0xFF )
		count = min( count, _length );

	U8	bufferIndex = m_bufferIndex - m_dataLength;
	for ( U8 i=count; i > 0; i-- ) {
		bufferIndex &= m_bufferIndexMask;
		*_data++ = m_ringBuffer[bufferIndex++];
	}
	m_dataLength -= count;
	sei();

	return count;
}


//////////////////////////////////////////////////////////////////////////
// General TWI interrupt handler
//
void	TWI::InterruptHandler() {
	U8	status = TWSR & 0xF8U;	// Discard low 3 bits
	if ( m_master ) {
		// Master mode
		if ( m_transmit )
			HandleMT( status );
		else
			HandleMR( status );
	} else {
		// Slave mode
		if ( m_transmit )
			HandleST( status );
		else
			HandleSR( status );
	}

	// TWINT flag in TWCR must be cleared at the end of the operation (data/address/status accesses must be complete before clearing the flag)
	// Apparently, it's automatically cleared when the interrupt returns
//	TWCR &= ~_BV(TWINT);
}

//////////////////////////////////////////////////////////////////////////
void	TWI::HandleMT( U8 _status ) {    
	switch ( _status ) {
	case 0x00:
		m_status = STATUS_ERROR;	// Bus error!
		break;

	case 0x08:	// Start received
	case 0x10:	// Repeated start received
		m_needsRestart = false;
		TWDR = (m_address << 1) | 0;	// Transmit address + Write flag
		TWCR = CTRL_ACK;
		break;

	case 0x18: // SLA+W received.
		m_status = STATUS_TRANSMITTING;
		// Fallthrough to transmitting data

	case 0x28:	// Data transmitted + ACK
		if ( m_dataLength == 0 ) {
			// Signal stop...
			TWCR = CTRL_ACK | _BV(TWSTO);
			m_status = STATUS_WAITING;
			break;
		}

		// Attempt transmitting another byte
		TWDR = m_ringBuffer[m_bufferIndex++];
		m_bufferIndex &= m_bufferIndexMask;
		m_dataLength--;
		TWCR = CTRL_NACK;
		break;

	case 0x20:	// SLA+W transmitted but NACK
	case 0x30:	// Data transmitted but NACK
		TWCR = CTRL_NACK | _BV(TWSTO);	// With the stop bit
	case 0x38:	// Master arbitration lost
		m_status = STATUS_ERROR;
		break;

	// Failed to enter Master Transmit
	case 0x68:	// Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
	case 0xB0:	// Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
	case 0x78:	// Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
		m_master = false;	// Now a slave to another master
		m_status = STATUS_WAITING;
		break;
	}
}
void	TWI::HandleMR( U8 _status ) {
	switch ( _status ) {
	case 0x00:
		m_status = STATUS_ERROR;	// Bus error!
		break;

	case 0x08:	// Start received
	case 0x10:	// Repeated start received
		m_needsRestart = false;
		TWDR = (m_address << 1) | 1;	// Transmit address + Read flag
		TWCR = CTRL_ACK;
		break;

	case 0x40:	// SLA+R has been transmitted; ACK has been received
		m_status = STATUS_RECEIVING;
//		TWCR = CTRL_ACK;
		break;

	case 0x58:	// Data byte has been received; NOT ACK has been returned
		m_status = STATUS_WAITING;
		TWCR = CTRL_NACK | _BV(TWSTO);	// With the stop bit
		// Fallthrough to receiving data

	case 0x50:	// Data byte has been received; ACK has been returned
		m_ringBuffer[m_bufferIndex++] = TWDR;
		m_bufferIndex &= m_bufferIndexMask;
		m_dataLength++;
		break;

	case 0x48:	// SLA+R has been transmitted; NOT ACK has been received
		TWCR = CTRL_NACK | _BV(TWSTO);	// With the stop bit
	case 0x38:	// Arbitration lost in SLA+R/W or NOT ACK bit
		m_status = STATUS_ERROR;
		break;

	// Failed to enter Master Receive
	case 0x68:	// Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
	case 0xB0:	// Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
	case 0x78:	// Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
		m_master = false;	// Now a slave to another master
		m_status = STATUS_WAITING;
		break;
	}
}
void	TWI::HandleST( U8 _status ) {
	// TODO
}
void	TWI::HandleSR( U8 _status ) {
	// TODO
}
