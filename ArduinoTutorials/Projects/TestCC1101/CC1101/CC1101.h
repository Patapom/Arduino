// 
//
namespace Pom {
	class CC1101 {

		byte	m_pin_CS;
		byte	m_pin_Clock;
		byte	m_pin_SI;
		byte	m_pin_SO;
		byte	m_pin_GDO0;
		byte	m_pin_GDO2;


		float	m_Fosc_MHz;			// Oscillation frequency in MHz (default is 26.0)

	public:
		struct Setup_t {
			float	Fosc_MHz;		// Oscillation frequency in MHz (default is 26.0)
			float	baseFrequency;	// Base operating frequency (default is 868MHz)
			byte	channel;		// Operating channel [0,255]
		};

	public:
		CC1101( byte _CS, byte _CLOCK, byte _SI, byte _SO, byte _GDO0, byte _GDO2 );
		void	Init( Setup_t _parms );

	private:
		void	SPIW( byte _value );	// Base SPI Write used by all other routines
		byte	SPIR();					// Base SPI Read used by all other routines

		byte	SPIReadSingle( byte _opcode );
		void	SPIReadBurst( byte _opcode, uint32_t _dataLength, byte* _data );
		void	SPIRead( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data );

		void	SPIWriteSingle( byte _opcode, byte _data );
		void	SPIWriteBurst( byte _opcode, uint32_t _dataLength, byte* _data );
		void	SPIWrite( byte _address, byte _opcodeOR, uint32_t _dataLength, byte* _data );

		void	Reset();													// Performs a manual reset
		void	SetChannelSpacing( float _spacing_KHz=1.0f );				// Sets the frequency spacing (in KHz) between channels
		void	SetFrequencyDeviation( float _deviation_KHz=47.607f );		// Sets the frequency deviation (in KHz) for phase shifts

		void	SetRegister( byte _address, byte _value );
		void	SendCommandStrobe( byte _command );
		void	SetPATable( byte _powerTable[8] );
		byte	ReadStatus();
		void	Burst( uint32_t _length, byte _data[] );
	};
}