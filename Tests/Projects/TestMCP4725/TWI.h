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
		STATUS_STOPPED,
		STATUS_TRANSMITTING,
		STATUS_RECEIVING,
		STATUS_ERROR,
	};

	static const U8		MAX_LENGTH_POT = 6;
	static const U32	MAX_LENGTH = 64;


private:

	static const U8	CTRL_ACK = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
	static const U8	CTRL_NACK = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);

	U8				m_transmitAddress;
	U8				m_ringBuffer[MAX_LENGTH];
	U8				m_bufferIndexMask;
	volatile U8		m_bufferIndex;
	volatile U8		m_dataLength;
	volatile STATUS	m_status;

public:

	TWI();
	~TWI();

	void	SetFrequency( U32 _frequencyMHz, PRESCALER_VALUE _prescaler );

	// Begins transmission for the target TWI slave af the provided address
	void	BeginTransmit( U8 _address );
	void	Push( U8* _data, U8 _length );	// Feeds data bytes to the TWI (warning: this is a ring buffer so if _length exceeds the buffer length, existing data will get overwritten!)
//	void	EndTransmit();

	// Don't call this yourself! Let the interrupt vector call it...
	void	InterruptHandler();
};