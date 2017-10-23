// 
//
namespace Pom {
	class CC1101 {

		byte	m_pin_CS;
		byte	m_pin_Clock;
		byte	m_pin_SI;
		byte	m_pin_SO;
		byte	m_pin_GD00;
		byte	m_pin_GD02;

	public:
		void	Init( byte _CS, byte _CLOCK, byte _SI, byte _SO, byte _GD00, byte _GD02 );

	private:
		void	SPISingle( bool _write, byte _opcode, byte _data );
		void	SPIBurst( bool _write, byte _opcode, uint32_t _dataLength, byte* _data );

		void	SetRegister( byte _address, byte _value );
		void	SendCommandStrobe( byte _address, byte _command );
		void	SetPATable( byte _powerTable[8] );
		void	ReadStatus();
		void	Burst( uint32_t _length, byte _data[] );
	};
}