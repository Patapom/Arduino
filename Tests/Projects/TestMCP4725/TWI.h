//////////////////////////////////////////////////////////////////////////
// Implements the Two-Wire Interface (TWI) protocol
//////////////////////////////////////////////////////////////////////////
//

class TWI {
public:

	enum PRESCALER_VALUE {
		PRESCALE_1 = 0,
		PRESCALE_4 = 1,
		PRESCALE_16 = 2,
		PRESCALE_64 = 3,
	};

	enum STATUS {
		STATUS_WAITING,
		STATUS_TRANSMITTING,
		STATUS_RECEIVING,
		STATUS_ERROR,
	};

	static const U8		MAX_LENGTH_POT = 6;
	static const U32	MAX_LENGTH = 64;


private:

	static const U8	CTRL_ACK = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);	// Enable TWI + Enable interrupt + Raise INT flag + ACK
	static const U8	CTRL_NACK = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);				// Enable TWI + Enable interrupt + Raise INT flag + NACK

	U8				m_transmitAddress;
	U8				m_ringBuffer[MAX_LENGTH];
	U8				m_bufferIndexMask;
	volatile U8		m_bufferIndex;
	volatile U8		m_dataLength;

	volatile STATUS	m_status : 6;
	volatile bool	m_master : 1;	// True = master, false = slace
	volatile bool	m_transmit : 1;	// True = transmit, false = receive


public:

	TWI();
	~TWI();

	void	SetFrequency( U32 _frequencyMHz, PRESCALER_VALUE _prescaler );

	void	BeginTransmit( U8 _address );	// Begins master transmission for the target TWI slave af the provided address
	void	BeginReply( U8 _address );		// Begins slave transmission for the target TWI master af the provided address
	void	Push( U8* _data, U8 _length );	// Feeds data bytes to the TWI (warning: this is a ring buffer so if _length exceeds the buffer length, existing data will get overwritten!)

public:
	// Don't call this yourself! Let the interrupt vector call it...
	void	InterruptHandler();
private:
	void	HandleMT( U8 _status );	// Handles master transmit mode
	void	HandleMR( U8 _status );	// Handles master receive mode
	void	HandleST( U8 _status );	// Handles slave transmit mode
	void	HandleSR( U8 _status );	// Handles slave receive mode
};