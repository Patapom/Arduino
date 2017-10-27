// 
//
namespace Pom {
	class CC1101 {

		const float		Fosc_MHz = 26.0f;	// Set to 26.0MHz

	public:
		struct Setup_t {
			float	carrierFrequency;	// Operating carrier frequency (default is 800MHz)
			byte	channel;			// Operating channel [0,255]
		};

		enum PACKET_LENGTH_CONFIG {
			FIXED = 0,		// Fixed packet length mode. Length configured manually
			VARIABLE = 1,	// Variable packet length mode where the packet's length is the first byte following the SYNC word
			INFINITE = 2,	// Infinite packet length where the user is responsible for switching to FIXED length when their 
		};

	private:
		// Internal pin configurations
		byte	m_pin_CS;
		byte	m_pin_Clock;
		byte	m_pin_SI;
		byte	m_pin_SO;
		byte	m_pin_GDO0;
		byte	m_pin_GDO2;

		// Internal state for PKTCTRL0 and 1
		bool	m_enableWhitening : 1;						// Enables data whitening (i.e. deterministic noise is added to transmitted data to both encrypt and increase quality of transfer)
		bool	m_useFIFO : 1;								// True to use default FIFO queues for RX and TX, false to use synchronous transmission mode
		bool	m_enableCRC : 1;							// Enables CRC (i.e. a Cyclic Redundancy Check sum is computed for the packet and transmitted at the end so the receiver can verify if the received packet was corrupted and discard it)
		PACKET_LENGTH_CONFIG	m_packetLengthConfig : 2;
		bool	m_enablePacketAddressCheck : 1;				// Enables address sending/checking at the beginning of packets (i.e. several slave chips can send to a single master and the master can dispatch to the proper slave given its address)

	public:
		CC1101( byte _CS, byte _CLOCK, byte _SI, byte _SO, byte _GDO0, byte _GDO2 );
		void	Init( Setup_t _parms );

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

		void	Reset();													// Performs a manual reset

	private:
		byte	SPITransfer( byte _value );	// Base SPI transfer used by all other routines

		byte	SPIReadSingle( byte _opcode );													// Returns READ byte (NOT! Status byte)
		byte	SPIReadBurst( byte _opcode, uint32_t _dataLength, byte* _data );				// Returns status byte
		byte	SPIRead( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data );	// Returns status byte

		byte	SPIWriteSingle( byte _opcode, byte _data );										// Returns status byte
		byte	SPIWriteBurst( byte _opcode, uint32_t _dataLength, byte* _data );				// Returns status byte
		byte	SPIWrite( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data );	// Returns status byte

		byte	SetRegister( byte _address, byte _value );
		byte	SendCommandStrobe( byte _command );
		void	SetPATable( byte _powerTable[8] );
		byte	ReadStatus();

		void	WritePKTCTRL0();
		void	ReadPKTCTRL0();
		void	WritePKTCTRL1();
		void	ReadPKTCTRL1();

	public:
		// DEBUG
		void	DisplayStatus( byte _status );
		void	DisplayDecodedWrittenValue( byte _writtenValue );	// Shows a decoded written value as a readable string
		void	DumpAllRegisters( byte _registerValues[0x3E] );		// Dumps all of the registers's current state
	};
}