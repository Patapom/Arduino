//////////////////////////////////////////////////////////////////////////
// Implements the Two-Wire Interface (TWI) protocol
//////////////////////////////////////////////////////////////////////////
//
// Here you can find the SDA/SCL pins depending on the Arduino board you're using
//	Board			I2C / TWI pins
//	Uno, Ethernet	A4 (SDA),	A5 (SCL)
//	Mega2560		20 (SDA),	21 (SCL)
//	Leonardo		 2 (SDA),	 3 (SCL)
//	Due				20 (SDA),	21 (SCL), SDA1, SCL1
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

	U8				m_address;
	U8				m_ringBuffer[MAX_LENGTH];
	U8				m_bufferIndexMask;
	volatile U8		m_bufferIndex;
	volatile U8		m_dataLength;

	volatile STATUS	m_status : 5;
	bool			m_master : 1;		// True = master, false = slace
	bool			m_transmit : 1;		// True = transmit, false = receive
	bool			m_needsRestart : 1;	// True = to send restart, false = to simply start


public:

	TWI( U8 _slaveAddress=0, bool _enableGeneralCall=false );
	~TWI();

	void	SetFrequency( U32 _frequencyMHz=400000U, PRESCALER_VALUE _prescaler=PRESCALE_1 );	// Default to 400KHz

	// Gets the current status
	STATUS	GetStatus() const;

	// Master Transmitter
	void	BeginTransmit( U8 _address );			// Begins master transmission to the target TWI slave af the provided address
//TODO	void	BeginReply( U8 _address );			// Begins slave transmission to the target TWI master af the provided address
	void	Push( const U8* _data, U8 _length );	// Feeds data bytes to the TWI (warning: this is a ring buffer so if _length exceeds the buffer length, existing data will get overwritten!)

	// Master Receiver
	void	BeginReceive( U8 _address );			// Begins master reception from the target TWI slave af the provided address
	U8		GetAvailableDataLength() const;			// Returns the length of available data
	U8		Pull( U8* _data, U8 _length=0xFF );		// Pulls the available data. Leave _length=0xFF to pull ALL available data. Returns the amount of actually pulled data

public:
	// Don't call this yourself! Let the interrupt vector call it...
	void	InterruptHandler();
private:
	void	HandleMT( U8 _status );	// Handles master transmit mode
	void	HandleMR( U8 _status );	// Handles master receive mode
	void	HandleST( U8 _status );	// Handles slave transmit mode
	void	HandleSR( U8 _status );	// Handles slave receive mode
};
