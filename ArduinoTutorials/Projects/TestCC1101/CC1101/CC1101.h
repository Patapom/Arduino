//////////////////////////////////////////////////////////////////////////
// CC1101 Driver Library for Arduino
// Code: Patapom
// Date: 2017/10/27
//
// Contains a very customizable library to drive the CC1101 Sub 1GHz RF Transceiver from Texas Instrument (http://www.ti.com/product/CC1101)
//
// This library resembles a lot to other CC1101 driver libs except it's much more verbose and contains lots of comments and descriptions
//	about how I understood the workings of the CC1101 chip
//
//////////////////////////////////////////////////////////////////////////
//

//#define SPI_DEBUG_VERBOSE	1	// Define to display Write/Read values on the SPI bus.
//#define SPI_DEBUG_VERBOSE	2	// Define to display Write/Read values on the SPI bus + decoding of the written value in plain text.

namespace Pom {
	class CC1101 {

		const float		Fosc_MHz = 26.0f;	// Set to 26.0MHz

	public:
		static const float	DEFAULT_CARRIER_FREQUENCY_MHz;// = 800.0f;

		struct Setup_t {
			float	carrierFrequency;	// Operating carrier frequency (default is 800MHz)
			byte	channel;			// Operating channel [0,255]
		};

		enum PACKET_LENGTH_CONFIG {
			FIXED = 0,		// Fixed packet length mode. Length configured manually
			VARIABLE = 1,	// Variable packet length mode where the packet's length is the first byte following the SYNC word
			INFINITE = 2,	// Infinite packet length where the user is responsible for switching to FIXED length when their last integer packet is sent
		};

		// Main Radio Control FSM State
		enum MACHINE_STATE {
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

	private:
		// Internal pin configurations
		byte	m_pin_CS;
		byte	m_pin_Clock;
		byte	m_pin_SI;
		byte	m_pin_SO;

		// Internal state for PKTCTRL0 and 1
		bool	m_enableWhitening : 1;							// Enables data whitening (i.e. deterministic noise is added to transmitted data to both encrypt and increase quality of transfer)
		bool	m_useFIFO : 1;									// True to use default FIFO queues for RX and TX, false to use synchronous transmission mode
		bool	m_enableCRC : 1;								// Enables CRC (i.e. a Cyclic Redundancy Check sum is computed for the packet and transmitted at the end so the receiver can verify if the received packet was corrupted and discard it)
		PACKET_LENGTH_CONFIG	m_packetLengthConfig : 2;		// Determines how the packet size is configured (fixed, variable size sent with packet, or infinite and user-controled)
		bool	m_enablePacketAddressCheck : 1;					// Enables address sending/checking at the beginning of packets (i.e. several slave chips can send to a single master and the master can dispatch to the proper slave given its address)

	public:
		CC1101( byte _CS, byte _CLOCK, byte _SI, byte _SO, byte _GDO0, byte _GDO2 );
		void	Init( Setup_t _parms );
		void	Reset();										// Performs a manual reset

		// Configures packets
		void	SetAddress( byte _address=0x00 );
		void	EnableWhitening( bool _enable=true );			// Enables data whitening (i.e. deterministic noise is added to transmitted data to both encrypt and increase quality of transfer)
		void	UseFIFO( bool _useFIFO=true );					// True to use default FIFO queues for RX and TX, false to use synchronous transmission mode
		void	EnableCRC( bool _enableCRC=true );				// Enables CRC (i.e. a Cyclic Redundancy Check sum is computed for the packet and transmitted at the end so the receiver can verify if the received packet was corrupted and discard it)
		void	SetPacketLengthConfig( PACKET_LENGTH_CONFIG _value=VARIABLE );// Sets the packet length configuration
		void	EnablePacketAddressCheck( bool _enable=false );	// Enables address sending/checking at the beginning of packets (i.e. several slave chips can send to a single master and the master can dispatch to the proper slave given its address)
		void	SetPacketLength( byte _length=0xFF );			// Sets the fixed packet length (used only if PACKET_LENGTH_CONFIG == FIXED)
		void	SetSyncWord( uint16_t _syncWord=0xD391 );		// Sets the SYNC word sent at the beginning of packets for identification

		// Channel selection
		void	SetChannel( byte _channel=0x00 );

		// Advanced frequency settings
		void	SetCarrierFrequency( float _Fcarrier_MHz=800.0f );			// Sets the carrier frequency (in MHz)
		void	SetIntermediateFrequency( float _F_KHz=380.859375f );		// Sets the intermediate frequency (in KHz) https://en.wikipedia.org/wiki/Intermediate_frequency
		void	SetFrequencyOffset( float _Foffset_KHz=0.0f );				// Sets the frequency offset (in KHz) added to the base frequency before being sent to the synthesizer. Range is from ±202 kHz by steps of 1.587KHz
		void	SetChannelBandwithAndDataRate( float _bandwidth_KHz=203.125f, float _dataRate_KBauds=115.05126953125f );	// Sets the bandwidth (in KHz) of each channel (WARNING: must NOT be larger than channel spacing!) and  the data rate (in KBauds)
		void	SetChannelSpacing( float _spacing_KHz=200.0f );				// Sets the frequency spacing (in KHz) between channels (we have a maximum of 256 channels, each of them this value appart) (WARNING: must NOT be smaller than bandwidth!)
		void	SetFrequencyDeviation( float _deviation_KHz=47.607421875f );// Sets the frequency deviation (in KHz) for frequency shift keying

		MACHINE_STATE	ReadFSMState();							// Returns the current state of the chip's internal Finite State Machine (FSM)

	private:
		byte	SPITransfer( byte _value );						// Base SPI transfer used by all other routines

		byte	SPIReadSingle( byte _opcode );													// Returns READ byte (NOT! Status byte)
		byte	SPIReadBurst( byte _opcode, uint32_t _dataLength, byte* _data );				// Returns status byte
		byte	SPIRead( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data );	// Returns status byte

		byte	SPIWriteSingle( byte _opcode, byte _data );										// Returns status byte
		byte	SPIWriteBurst( byte _opcode, uint32_t _dataLength, byte* _data );				// Returns status byte
		byte	SPIWrite( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data );	// Returns status byte

		byte	SetRegister( byte _address, byte _value );										// Returns status byte
		byte	SendCommandStrobe( byte _command );												// Returns status byte
		byte	ReadPATable( byte _powerTable[8] );												// Returns status byte
		byte	WritePATable( byte _powerTable[8] );											// Returns status byte
		byte	ReadStatus();
		byte	ReadStatusRegister( byte _address );

		void	WritePKTCTRL0();
		void	ReadPKTCTRL0();
		void	WritePKTCTRL1();
		void	ReadPKTCTRL1();

		#ifdef SPI_DEBUG_VERBOSE
			public:
				// DEBUG
				void	DisplayStatus( byte _status );
				void	DisplayDecodedWrittenValue( byte _writtenValue );	// Shows a decoded written value as a readable string
				void	DumpAllRegisters( byte _registerValues[0x3E] );		// Dumps all of the registers's current state
		#endif
	};
}