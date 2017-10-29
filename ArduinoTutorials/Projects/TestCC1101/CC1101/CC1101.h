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

		enum PACKET_LENGTH_CONFIG {
			FIXED = 0,		// Fixed packet length mode. Length configured manually
			VARIABLE = 1,	// Variable packet length mode where the packet's length is the first byte following the SYNC word
			INFINITE = 2,	// Infinite packet length where the user is responsible for switching to FIXED length when their last integer packet is sent
		};

		enum PACKET_FORMAT {
			NORMAL = 0,			// Normal mode, use FIFO for RX and TX
			SYNCHRONOUS = 1,	// Synchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins
			ASYNCHRONOUS = 3,	// Asynchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins
		};

	private:
		// Internal pin configurations
		U8						m_pin_CS;
		U8						m_pin_Clock;
		U8						m_pin_SI;
		U8						m_pin_SO;
		U8						m_pin_GDO0;
		U8						m_pin_GDO2;

		// Internal state for PKTCTRL0 and 1
		bool					m_enableWhitening : 1;			// Enables data whitening (i.e. deterministic noise is added to transmitted data to both encrypt and increase quality of transfer)
		PACKET_FORMAT			m_packetFormat : 2;				// Packet format
		bool					m_enableCRC : 1;				// Enables CRC (i.e. a Cyclic Redundancy Check sum is computed for the packet and transmitted at the end so the receiver can verify if the received packet was corrupted and discard it)
		PACKET_LENGTH_CONFIG	m_packetLengthConfig : 2;		// Determines how the packet size is configured (fixed, variable size sent with packet, or infinite and user-controled)
		bool					m_enablePacketAddressCheck : 1;	// Enables address sending/checking at the beginning of packets (i.e. several slave chips can send to a single master and the master can dispatch to the proper slave given its address)

	public:
		CC1101( U8 _pin_CS, U8 _pin_CLOCK, U8 _pin_SI, U8 _pin_SO, U8 _pin_GDO0, U8 _pin_GDO2 );
		void	Reset();										// Performs a manual reset

		// Data transfer
		void	SetNormalTransferMode();						// Normal transfer mode (Synchronous mode + using FIFO)
		void	SetSynchronousTransferMode();					// Synchronous transfer mode (no FIFO: direct read/write on GDOx pins)
		void	SetAsynchronousTransferMode();					// Asynchronous transfer mode (no support, MCU must be polling and feeding data at oversampled rate on GDOx pins)

		void	Transmit( U8 _size, U8* _data );				// Transmits a small amount of data (< 256 bytes)
		U8		Receive( U8* _data );							// Reads a small amount of data (< 256 bytes). Returns 0 if nothing is present in the pipe.

		// Configures packets
		void	SetAddress( U8 _address=0x00 );
		void	EnableWhitening( bool _enable=true );			// Enables data whitening (i.e. deterministic noise is added to transmitted data to both encrypt and increase quality of transfer)
		void	SetPacketFormat( PACKET_FORMAT _format=NORMAL );// Specifies packet format to use
		void	EnableCRC( bool _enableCRC=true );				// Enables CRC (i.e. a Cyclic Redundancy Check sum is computed for the packet and transmitted at the end so the receiver can verify if the received packet was corrupted and discard it)
		void	SetPacketLengthConfig( PACKET_LENGTH_CONFIG _value=VARIABLE );// Sets the packet length configuration
		void	EnablePacketAddressCheck( bool _enable=false );	// Enables address sending/checking at the beginning of packets (i.e. several slave chips can send to a single master and the master can dispatch to the proper slave given its address)
		void	SetPacketLength( U8 _length=0xFF );				// Sets the fixed packet length (used only if PACKET_LENGTH_CONFIG == FIXED)
		void	SetSyncWord( U16 _syncWord=0xD391 );			// Sets the SYNC word sent at the beginning of packets for identification

		// Channel selection
		void	SetChannel( U8 _channel=0x00 );

		// Advanced frequency settings
		float	GetCarrierFrequency();										// Gets the carrier frequency (in MHz)
		void	SetCarrierFrequency( float _Fcarrier_MHz=867.999939f );		// Sets the carrier frequency (in MHz)
		float	GetIntermediateFrequency();									// Sets the intermediate frequency (in KHz)
		void	SetIntermediateFrequency( float _F_KHz=203.125f );			// Sets the intermediate frequency (in KHz) https://en.wikipedia.org/wiki/Intermediate_frequency
		float	GetFrequencyOffset();										// Sets the frequency offset (in KHz)
		void	SetFrequencyOffset( float _Foffset_KHz=0.0f );				// Sets the frequency offset (in KHz) added to the base frequency before being sent to the synthesizer. Range is from ±202 kHz by steps of 1.587KHz
		void	GetChannelBandwithAndDataRate( float& _bandwidth_KHz, float& _dataRate_KBauds );
		void	SetChannelBandwithAndDataRate( float _bandwidth_KHz=325.0f, float _dataRate_KBauds=99.9756f );	// Sets the bandwidth (in KHz) of each channel (WARNING: must NOT be larger than channel spacing!) and  the data rate (in KBauds)
		float	GetChannelSpacing();										// Sets the frequency spacing (in KHz) between channels
		void	SetChannelSpacing( float _spacing_KHz=199.951172f );		// Sets the frequency spacing (in KHz) between channels (we have a maximum of 256 channels, each of them this value appart) (WARNING: must NOT be smaller than bandwidth!)
		float	GetFrequencyDeviation();									// Gets the frequency deviation (in KHz) for frequency shift keying
		void	SetFrequencyDeviation( float _deviation_KHz=47.607421875f );// Sets the frequency deviation (in KHz) for frequency shift keying

		// Configure how General Purpose Pins (GDOx) should behave
		enum GDO_SELECT {
			GDO0,
			// NOTE: GDO1 cannot be selected as it is the SO pin.
			GDO2,
		};
		enum GDO_CONFIG {
			RX_FIFO_ASSERT_FULL = 0,					// Associated to the RX FIFO: Asserts when RX FIFO is filled at or above the RX FIFO threshold. De-asserts when RX FIFO is drained below the same threshold.
			RX_FIFO_ASSERT_FULL_OR_PACKET_END = 1,		// Associated to the RX FIFO: Asserts when RX FIFO is filled at or above the RX FIFO threshold or the end of packet is reached. De-asserts when the RX FIFO is empty.
			TX_FIFO_ASSERT_FULL_OR_ABOVE_THRESHOLD = 2,	// Associated to the TX FIFO: Asserts when the TX FIFO is filled at or above the TX FIFO threshold. De-asserts when the TX FIFO is below the same threshold.
			TX_FIFO_ASSERT_FULL = 3,					// Associated to the TX FIFO: Asserts when TX FIFO is full. De-asserts when the TX FIFO is drained below the TX FIFO threshold.
			RX_FIFO_OVERFLOW = 4,						// Asserts when the RX FIFO has overflowed. De-asserts when the FIFO has been flushed.
			TX_FIFO_UNDERFLOW = 5,						// Asserts when the TX FIFO has underflowed. De-asserts when the FIFO is flushed.
			ASSERT_ON_SYNC_WORD = 6,					// Asserts when sync word has been sent / received, and de-asserts at the end of the packet.
														//	• In RX, the pin will also deassert when a packet is discarded due to address or maximum length filtering or when the radio enters RXFIFO_OVERFLOW state.
														//	• In TX the pin will de-assert if the TX FIFO underflows.
			ASSERT_ON_PACKET_RECEIVED_OK = 7,			// Asserts when a packet has been received with CRC OK. De-asserts when the first byte is read from the RX FIFO.
			PQI_ABOVE_PROGRAMMED_QUALITY = 8,			// Preamble Quality Reached. Asserts when the PQI is above the programmed PQT value. De-asserted when the chip reenters RX state (MARCSTATE=0x0D) or the PQI gets below the programmed PQT value.
			RSSI_LEVEL_BELOW_THRESHOLD = 9,				// Clear channel assessment. High when RSSI level is below threshold (dependent on the current CCA_MODE setting).
			PLL_LOCKED = 10,							// Lock detector output. The PLL is in lock if the lock detector output has a positive transition or is constantly logic high. To check for PLL lock the lock detector output should be used as an interrupt for the MCU.
			SERIAL_CLOCK = 11,							// Serial Clock. Synchronous to the data in synchronous serial mode.
														//	• In RX mode, data is set up on the falling edge by CC1101 when GDOx_INV=0.
														//	• In TX mode, data is sampled by CC1101 on the rising edge of the serial clock when GDOx_INV=0.
			SYNCHRONOUS_DATA_OUT = 12,					// Serial Synchronous Data Output. Used for synchronous serial mode.
			ASYNCHRONOUS_DATA_OUT = 13,					// Serial Data Output. Used for asynchronous serial mode.
			RSSI_LEVEL_ABOVE_THRESHOLD = 14,			// Carrier sense. High if RSSI level is above threshold. Cleared when entering IDLE mode.
			CRC_OK = 15,								// CRC_OK. The last CRC comparison matched. Cleared when entering/restarting RX mode.
			WOR_EVNT0 = 36,								// Wake On Radio Event 0
			WOR_EVNT1 = 37,								// Wake On Radio Event 1
			CLK_256 = 38,								// CLK_256 (undocumented)
			CLK_32k = 39,								// CLK_32k (undocumented)
			CHIP_RDYn = 41,								// Signal chip is ready (crystal is running) (default value after a reset)
			XOSC_STABLE = 43,							// Oscillator is stable (should normally be at the same time as CHIP_RDYn)
			HIGH_IMPEDANCE = 46,						// High impedance (3-state) (default value for SO/GDO1)
			HW_TO_0 = 47,								// HW to 0 (HW1 achieved by setting GDOx_INV=1). Can be used to control an external LNA/PA or RX/TX switch.

			// Note: There are 3 GDO pins, but only one CLK_XOSC/n can be selected as an output at any time.
			// If CLK_XOSC/n is to be monitored on one of the GDO pins, the other two GDO pins must be configured to values less than 0x30.
			// The GDO0 default value is CLK_XOSC/192. [Patapom NOTE: I actually modified this so GDO0 is CHIP_RDYn after Reset() is called for the reason described below]
			// To optimize RF performance, these signals should not be used while the radio is in RX or TX mode.
			CLK_XOSC_1		= 48,	// Same frequency as chip's clock (26MHz default)
			CLK_XOSC_1_5	= 49,
			CLK_XOSC_2		= 50,
			CLK_XOSC_3		= 51,
			CLK_XOSC_4		= 52,
			CLK_XOSC_6		= 53,
			CLK_XOSC_8		= 54,
			CLK_XOSC_12		= 55,
			CLK_XOSC_16		= 56,
			CLK_XOSC_24		= 57,
			CLK_XOSC_32		= 58,
			CLK_XOSC_48		= 59,
			CLK_XOSC_64		= 60,
			CLK_XOSC_96		= 61,
			CLK_XOSC_128	= 62,
			CLK_XOSC_192	= 63,	// Clock frequency divided by 192
		};
		void	SetGDOx( GDO_SELECT _GDO, GDO_CONFIG _config=CHIP_RDYn, bool _invertOutput=false );	// If _GDO0 is true, sets GDO0. Sets GDO2 if false.

		// Returns the current state of the chip's internal Main Radio Control Finite State Machine (FSM)
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
		MACHINE_STATE	ReadFSMState();

	private:
		U8		SPITransfer( U8 _value );										// Base SPI transfer used by all other routines

		U8		SPIReadSingle( U8 _opcode );									// Returns READ byte (NOT! Status byte)
		U8		SPIReadBurst( U8 _opcode, uint32_t _dataLength, U8* _data );	// Returns status byte
		U8		SPIRead( U8 _address, uint32_t _dataLength, U8* _data );		// Returns status byte

		U8		SPIWriteSingle( U8 _opcode, U8 _data );							// Returns status byte
		U8		SPIWriteBurst( U8 _opcode, uint32_t _dataLength, U8* _data );	// Returns status byte
		U8		SPIWrite( U8 _address, uint32_t _dataLength, U8* _data );		// Returns status byte

		U8		GetRegister( U8 _address );										// Returns READ byte (NOT! Status byte)
		U8		SetRegister( U8 _address, U8 _value );							// Returns status byte
		U8		SendCommandStrobe( U8 _command );								// Returns status byte
		U8		ReadPATable( U8 _powerTable[8] );								// Returns status byte
		U8		WritePATable( U8 _powerTable[8] );								// Returns status byte
		U8		ReadStatus();
		U8		ReadStatusRegister( U8 _address );

		void	WritePKTCTRL0();
		void	ReadPKTCTRL0();
		void	WritePKTCTRL1();
		void	ReadPKTCTRL1();

		void	InternalCustomReset();							// Some internal custom reset operations that are executed right after Reset() is called

		void	DumpManyStates( U8 _stateRegister, U64 _startTime, U16 _count=2048 );
		void	DisplayStatusRegisters();

		#ifdef SPI_DEBUG_VERBOSE
			public:
				// DEBUG
				void	DisplayStatus( U8 _status );
				void	DisplayDecodedWrittenValue( U8 _writtenValue );	// Shows a decoded written value as a readable string
				void	DumpAllRegisters( U8 _registerValues[0x3E] );	// Dumps all of the registers's current state
		#endif
	};
}