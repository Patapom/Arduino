////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// nRF24L01 Simple driver library
// 
// ISSUES:
//  • When not using IRQs and using the SendPacket() function, we very often have a TR_FAILED result although the packet actually DID transfer!
//    => This happens because a MAX_RT bit was raised so we clearly have to flush the TX pipe, but this also prevents us to know if the packet
//        was really received and ACK was sent back!
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include "Global.h"

class nRF24L01 {
  public: // TYPES

    enum ENABLE_PIPES {
      NO_PIPE = 0,
      PIPE0 = 1,
      PIPE1 = 2,
      PIPE2 = 4,
      PIPE3 = 8,
      PIPE4 = 16,
      PIPE5 = 32,
      ALL_PIPES = 0x3F,
    };

    enum DATA_RATE {
      DR_1MBPS = 0,
      DR_2MBPS = 1
    };

    enum POWER_LEVEL {
      PL_18dBm = 0, // -18 dBm 7.0mA  
      PL_12dBm = 1, // -12dBm 7.5mA
      PL_6dBm  = 2, // -6dBm 9.0mA
      PL_0dBm  = 3, // 0dBm 11.3mA
    };

    enum TRANSMIT_RESULT {
      TR_OK = 0,    // ACK received
      TR_FAILED,    // Maximum restransmits was reached without any ACK signal received (you need to clear the TX FIFO manually)
      TR_FIFO_EMPTY,// TX FIFO is empty (nothing to transmit)
      TR_FIFO_FULL, // TX FIFO is full
    };

    enum RECEIVE_RESULT {
      RR_OK = 0,
      RR_TIMEOUT,   // Failed to receive packet before elapsed time
      RR_INCONSISTENT_FIFO_STATE, // The FIFO is in an inconsistent state: we were notified a packet arrived but the data pipe index is invalid...
    };

  public:  // FIELDS
    U8  m_pinCE, m_pinCSN;
    U8  m_status;

public:

  nRF24L01( U8 _pinCE, U8 _pinCSN ) {
    m_pinCE = _pinCE;
    m_pinCSN = _pinCSN;

    pinMode( m_pinCE, OUTPUT );
    pinMode( m_pinCSN, OUTPUT );

    Disable();  // Standby
    Deselect(); // Deselect chip
  }

  bool  Init( ENABLE_PIPES _pipeFlags ) {
    SPI.begin();
    SPI.beginTransaction( SPISettings( 4000000, MSBFIRST, SPI_MODE0 ) );  // 1MHz? Check that in the doc...
//  SPI.setBitOrder( LSBFIRST ); WRONG!
//  SPI.setClockDivider( SPI_CLOCK_DIV8 );
    SPDR = 0;

    // Allow module time to settle
    delay( 5 );

//delay( 1000 );

//Serial.println( str( "Status %02X", m_status ) );
//Serial.println( str( "NRF_CONFIG = 0x%02X", Read( NRF_CONFIG ) ) );
//PRX();
//    Write( NRF_CONFIG, 0x0E );  // Enable all IRQs, enable CRC, 2-bytes CRC, Power Up
//Serial.println( str( "NRF_CONFIG = 0x%02X", Read( NRF_CONFIG ) ) );
//PTX();
//Serial.println( str( "NRF_CONFIG = 0x%02X", Read( NRF_CONFIG ) ) );
//PRX();
//Serial.println( str( "NRF_CONFIG = 0x%02X", Read( NRF_CONFIG ) ) );

    Write( EN_AA, 0x3F );   // Enable auto-ACK for all pipes (PTX must receive ACK after transmitting a packet)
    Write( EN_RXADDR, _pipeFlags );  // Enable RX addresses for specified pipes only
    Write( RX_PW_P0, 32 );  // Payload length for pipe 0 = 32 bytes
    Write( RX_PW_P1, 32 );  // Payload length for pipe 1 = 32 bytes
    Write( RX_PW_P2, 32 );  // Payload length for pipe 2 = 32 bytes
    Write( RX_PW_P3, 32 );  // Payload length for pipe 3 = 32 bytes
    Write( RX_PW_P4, 32 );  // Payload length for pipe 4 = 32 bytes
    Write( RX_PW_P5, 32 );  // Payload length for pipe 5 = 32 bytes
    Write( SETUP_AW, 3 );   // Address width = 5 bytes

    // I couldn't see any major improvements on the success rate of packets when increasing the amount of retransmits to the max...
//    Write( SETUP_RETR, 0x5F );  // Setup automatic retransmission ARD:4 = Wait 1500µs | ARC:4 = Retransmit 15 times
    Write( SETUP_RETR, 0x03 );  // Setup automatic retransmission ARD:4 = Wait 250µs | ARC:4 = Retransmit 3 times

    // Central channel
    SetChannel( 76 );

    // Clear any residual state from previous use
    ClearInterruptFlags();

    FlushRX();
    FlushTX();

//  Serial.println( str( "SPCR = 0x%02X", SPCR ) ); // 0x50

    PRX();

    return Read( NRF_CONFIG ) == 0x0F;  // Was configuration written properly? Are we in receiver mode?
  }

  void  PRX() { Write( NRF_CONFIG, 0x0E | _BV(PRIM_RX) ); } // Configure as receiver (0 + !MASK_RX_DR + !MASK_TX_DS + !MASK_MAX_RT | EN_CRC + CRCO = 2 bytes CRC + PWR_UP + PRIM_RX)
  void  PTX() { Write( NRF_CONFIG, 0x0E ); }                // Configure as transmitter

  // Sets the air data rate and power level of the device
  void  SetAirDataRateAndPower( DATA_RATE _dataRate, POWER_LEVEL _power ) {
    U8  value = !_BV( PLL_LOCK ) | _BV( LNA_HCURR ) | (_power << 1) | (_dataRate << 3); // [06] RF_SETUP default = 0x01 => 000 + !PLL_LOCK = Don't force PLL signal (only use in test) | RF_DR:1 = 1Mbps Air data rate + RF_PWR:2 = -18dBm + LNA_HCURR:1
    Write( RF_SETUP, value );
  }

  // Set the channel to communicate with [0,127]
  void  SetChannel( U8 _channel ) {
    ERROR( _channel >= 128, "SetChannel", "Channel index out of range!" );
    Write( RF_CH, _channel & 0x7F );  // [05] RF_CH = 0 + RF_CH:7 = frequency channel (default = 0x4C)
  }

  // Sets the address of the transmitter
  // There's no need to set the target addresse before each transmission if the target didn't change
  void  SetTransmitterAddress( U8 _address[5] ) {
    Write( TX_ADDR, 5, _address );
    Write( RX_ADDR_P0, 5, _address ); // In transmitter mode, data pipe 0 must also have the same address for when we receive ACK
  }

  // Sets the single-receiver pipe address
  void  SetReceiverSingleAddress( U8 _address[5] ) {
    Write( RX_ADDR_P0, 5, _address );
  }

  // Sets the multi-receiver pipe address
  //  _address0, the 5-bytes address for pipe 0
  //  _baseAddress, the base 4 MSBytes for all 5 remaining pipe addresses
  //  _LSB1-5, the LSByte for each of the 5 remaining pipes
  void  SetReceiverMultipleAddresses( U8 _address0[5], U8 _baseAddress[4], U8 _LSB1, U8 _LSB2, U8 _LSB3, U8 _LSB4, U8 _LSB5 ) {
//    ERROR( _pipe > 5, "SetReceiverPipeAddress", "Pipe index out of range" );
    Write( RX_ADDR_P0, 5, _address0 );

    U8  address[5];
    memcpy( address, _baseAddress, 4 );
    
    address[4] = _LSB1; Write( RX_ADDR_P1, 5, address );
    address[4] = _LSB2; Write( RX_ADDR_P2, 5, address );
    address[4] = _LSB3; Write( RX_ADDR_P3, 5, address );
    address[4] = _LSB4; Write( RX_ADDR_P4, 5, address );
    address[4] = _LSB5; Write( RX_ADDR_P5, 5, address );
  }

  // Sends a packet, payload must be 32 bytes max
  TRANSMIT_RESULT SendPacket( U8 _payloadLength, U8* _payload ) {
    // Push a new packet
    if ( !PushTXPacket( _payloadLength, _payload ) )
      return TR_FIFO_FULL;

    return SendSingle();
  }

  // Waits to receive a packet or the time runs out. Must be in PRX mode to work!
  //  _pipeIndex, the index of the data pipe who sent the packet
  //  _payloadLength, the length of the received payload
  //  _payload, the received payload
  RECEIVE_RESULT  ReceivePacketWait( U8& _pipeIndex, U8& _payloadLength, U8* _payload, U32 _timeOut_ms=1000 ) {

    Enable(); // Start RX mode (130µs delay before communication starts)

    // Wait for a packet or a timeout
    U32 startTime = millis();
    U8  packetReceivedPipeIndex = 0xFF;
    while ( (packetReceivedPipeIndex = PacketReceived()) == 0xFF && (millis() - startTime) < _timeOut_ms ) {}
    if ( packetReceivedPipeIndex == 0xFF )
      return RR_TIMEOUT;  // Didn't receive anything in the imparted time...
    else if ( packetReceivedPipeIndex == 0xFE )
      return RR_INCONSISTENT_FIFO_STATE;  // Data received but invalid data pipe!

    Disable();  // Stop RX mode and go back to Standby-I (low current) => Can then go to TX, RX or Power down

    // Read the received payload
    _pipeIndex = packetReceivedPipeIndex;
    ReadPayload( _payloadLength, _payload );

    // Clear the "Data Received" IRQ bit 
    ClearInterruptFlags( _BV(RX_DR) );

    return RR_OK;
  }

public: // Helpers

  bool  IsMaxRT() { return Status() & _BV(MAX_RT); }
  bool  IsTXFIFOFull() { return Status() & _BV(TX_FULL); }
//  bool  IsTXFIFOFull() { return FIFOStatus() & _BV(FIFO_FULL); }  // Be careful NOT to use TX_FULL, which is bit 0 in NRF_STATUS register!
  bool  IsTXFIFOEmpty() { return FIFOStatus() & _BV(TX_EMPTY); }
  bool  IsRXFIFOFull() { return FIFOStatus() & _BV(RX_FULL); }
  bool  IsRXFIFOEmpty() { return FIFOStatus() & _BV(RX_EMPTY); }

  U8    GetLostPacketCount() { return ObserveTX() >> 4; }
  U8    GetRetransmittedPacketsCount() { return ObserveTX() & 0x0F; }

  // Pushes a new packet to the TX FIFO, payload must be 32 bytes max
  // Returns true if successful, false if FIFO is full
  bool  PushTXPacket( U8 _payloadLength, U8* _payload ) {
    if ( IsTXFIFOFull() )
        return false;

    U8  padLength = 32 - _payloadLength;

    Select();

    //  TX_PLD = [1-32] bytes (bytes comptés par le chip, pas besoin de spécifier)
    //    => Ecrire séquentiellement pendant que CSN low
    m_status = SPI.transfer( W_TX_PAYLOAD );

    while ( _payloadLength-- ) {
      SPI.transfer( *_payload++ );
    }
    while ( padLength-- ) {
      SPI.transfer( 0x00 );
    }

    Deselect();

    return true;
  }

  // Send any packet waiting in the TX FIFO. Must be in PTX mode to work!
  TRANSMIT_RESULT SendSingle() {
    if ( IsTXFIFOEmpty() )
      return TR_FIFO_EMPTY; // Nothing to transmit anyway...

#if 1
    Enable();

    PACKET_SENT_STATE pss = PSS_RETRANSMITTING;
    while ( pss == PSS_RETRANSMITTING ) {
      pss = PacketSent();
    }

    Disable();

#else
// Apparently, this is too short a pulse and we lose many pakets this way because the TX_DS flag is never raised...
// Prefer the above method that waits until TX_DS or MAX_RT flags are set in the status register...
//
    // CE = High (min 10µs) => Starts transmission
    PulseEnable( 10 );

    PACKET_SENT_STATE pss = PSS_RETRANSMITTING;
    while ( pss == PSS_RETRANSMITTING ) {
      pss = PacketSent();
    }
#endif

    // Clear interrupt flags
// @TODO: Apparently, official Arduino library says it's not necessary?
//	But at the same time, if we don't clear the TX_DS bit then we'd immediately exit the waiting loop above on next packet send with a false positive...
    ClearInterruptFlags();

    if ( pss == PSS_SENT )
      return TR_OK; // Sent!

    ERROR( pss != PSS_MAX_RETRANSMIT, "SendSingle", "Unexpected PSS!" );

    // Packet failed to send after too many retransmits...
    // We must flush the TX FIFO ourselves...
    FlushTX();

    return TR_FAILED;
  }

  // Send all packets waiting in the TX FIFO. Must be in PTX mode to work!
  TRANSMIT_RESULT SendAll( U8& _packetsLostCounter ) {
    if ( IsTXFIFOEmpty() )
      return TR_FIFO_EMPTY; // Nothing to transmit anyway...

    Enable();

    while ( !IsTXFIFOEmpty() ) {
      PACKET_SENT_STATE pss = PSS_RETRANSMITTING;
      while ( pss == PSS_RETRANSMITTING ) {
        pss = PacketSent();
      }

      ClearInterruptFlags();

      if ( pss == PSS_SENT )
        continue; // Wait for next packet to be transmitted...

      ERROR( pss != PSS_MAX_RETRANSMIT, "SendAll", "Unexpected PSS!" );

      // Packet failed to send after too many retransmits...
      // We must flush the TX FIFO ourselves...
      FlushTX();
    }

    Disable();

    return TR_OK;
  }

/* From RF24 library
bool RF24::txStandBy() {

    uint32_t timeout = millis();
    while (!(read_register(FIFO_STATUS) & _BV(TX_EMPTY))) {
        if (status & _BV(MAX_RT)) {
            write_register(NRF_STATUS, _BV(MAX_RT));
            ce(LOW);
            flush_tx(); //Non blocking, flush the data
            return 0;
        }
        if (millis() - timeout > 95) {
            errNotify();
            return 0;
        }
    }

    ce(LOW); //Set STANDBY-I mode
    return 1;
}
*/

//protected:
public:

  U8    Status()      { return Read( NRF_STATUS ); }	// [07] NRF_STATUS = 0x0E 0 + !RX_DR + !TX_DS + !MAX_RT | RX_P_NO:3 = RX FIFO Empty (otherwise pipe # in [0,5]) + !TX_FULL
  U8    FIFOStatus()  { return Read( FIFO_STATUS ); }
  U8    ObserveTX()   { return Read( OBSERVE_TX ); }  // [08] OBSERVE_TX = 0x00 PLOS_CNT:4 = count lost packeges | ARC_CNT:4 = count retransmitted packages

  void  ClearInterruptFlags( U8 _flags = _BV(MAX_RT) | _BV(RX_DR) | _BV(TX_DS) ) {
    Write( NRF_STATUS, _flags );
  }

  // Returns 0xFF if no packet was received, or the index of the data pipe [0,5] in which the packet is available
  // Also returns 0xFE if a packet was received but the RX_FIFO is empty, which is indicative of an invalid state??
  U8    PacketReceived() {
    U8  status = Status();
    if ( (status & _BV( RX_DR )) == 0 )
      return 0xFF;

    U8  pipeIndex = (status & 0x0F) >> 1;
    if ( pipeIndex > 5 )
      return 0xFE;  // FIFO empty although packet was received???

    return pipeIndex;
  }

  enum PACKET_SENT_STATE {
    PSS_SENT,           // Packet was sent and received successfully
    PSS_RETRANSMITTING, // Packet is not yet acknowledged and is being retransmitted
    PSS_MAX_RETRANSMIT, // Packet was sent too many times without being acknowledged
  };

  PACKET_SENT_STATE PacketSent() {
    U8  status = Status();
    if ( status & _BV( TX_DS ) )
      return PSS_SENT;  // Success!
    if ( status & _BV( MAX_RT ) )
      return PSS_MAX_RETRANSMIT;  // Failed too many times...

    return PSS_RETRANSMITTING;  // Still not acknowledged...
  }

  // Flush functions => the entire RX/TX pipe is flushed! So if you have 3 payloads and your FIFO is full, everything is flushed at once and the FIFO is completely empty...
  void  FlushRX() { Write( FLUSH_RX ); }
  void  FlushTX() { Write( FLUSH_TX ); }

  void  Deselect() { Select( false ); }
  void  Select( bool _select=true ) { digitalWrite( m_pinCSN, !_select ); }
  void  Disable() { Enable( false ); }
  void  Enable( bool _enable=true ) { digitalWrite( m_pinCE, _enable ); }
  void  PulseEnable( U32 _delay_microseconds ) {
    Enable();
    delayMicroseconds( _delay_microseconds );
    Disable();
  }

public:
void  DumpRegisters() {
  char* registerNames[] = {
"NRF_CONFIG",	// [00]  = 0x0E 0 + !MASK_RX_DR + !MASK_TX_DS + !MASK_MAX_RT | EN_CRC + CRCO = 2 bytes CRC + PWR_UP + PRIM_RX = PRX
"EN_AA     ",	// [01]  = 0x3F 00 + Enable pipe 5, 4 | Enable pipe 3, 2, 1, 0    (Auto-Acknowledgement for "Enhanced ShockBurst")
"EN_RXADDR ",	// [02]  = 0x03 00 + Disable pipe 5, 4 | Disable pipe 3, 2 + Enable pipe 1, 0 (Enable RX addresses)
"SETUP_AW  ",	// [03]  = 0x03 0000 | 00 + AW:2 = 5 bytes Address Width (can be 01=3 bytes, 10=4 bytes or 11=5 bytes)
"SETUP_RETR",	// [04]  = 0x5F ARD:4 = Wait 1500µs | ARC:4 = Retransmit 15 times (Setup automatic retransmission)
"RF_CH     ",	// [05]  = 0x4C 0 + RF_CH:7 = frequency channel
"RF_SETUP  ",	// [06]  = 0x01 000 + !PLL_LOCK = Don't force PLL signal (only use in test) | RF_DR:1 = 1Mbps Air data rate + RF_PWR:2 = -18dBm + LNA_HCURR:1
"NRF_STATUS",	// [07]  = 0x0E 0 + !RX_DR + !TX_DS + !MAX_RT | RX_P_NO:3 = RX FIFO Empty (otherwise pipe # in [0,5]) + !TX_FULL
"OBSERVE_TX",	// [08]  = 0x00 PLOS_CNT:4 = count lost packeges | ARC_CNT:4 = count retransmitted packages
"CD        ",	// [09]  = 0x00 0000 | 000 + CS = Carrier Detect
//"RX_ADDR_P0",	// [0A]  = 0x3030303031   Receive address data pipe 0. 5 Bytes maximum length. (LSByte is written first. Write the number of bytes defined by SETUP_AW)
//"RX_ADDR_P1",	// [0B]  = 0xC2C2C2C2C2          -                  1
"RX_ADDR_P2",	// [0C]  = 0xC3           LSByte for pipe 2 (MSBytes are the same as RX_ADDR_P1)
"RX_ADDR_P3",	// [0D]  = 0xC4                  -        3
"RX_ADDR_P4",	// [0E]  = 0xC5                  -        4
"RX_ADDR_P5",	// [0F]  = 0xC6           LSByte for pipe 5
//"TX_ADDR   ",	// [10]  = 0xE7E7E7E7E7  Transmit address. Used for a PTX device only. Set RX_ADDR_P0 equal to this address to handle automatic acknowledge if this is a PTX device with Enhanced ShockBurst™ enabled.
"RX_PW_P0  ",	// [11]  = 0x20 00 + RX_PW_P0:6 = Number of bytes in RX payload in data pipe 0
"RX_PW_P1  ",	// [12]  = 0x20 Same for pipe 1
"RX_PW_P2  ",	// [13]  = 0x20       -       2
"RX_PW_P3  ",	// [14]  = 0x20       -       3
"RX_PW_P4  ",	// [15]  = 0x20       -       4
"RX_PW_P5  ",	// [16]  = 0x20       -       5
"FIFO_STATUS",// [17]  = 0x11 0 + !TX_REUSE + !FIFO_FULL + TX_EMPTY | 00 + !RX_FULL + RX_EMPTY
"DYNPD     ",	// [1C]  = 0x00 00 + !DPL_P5 + !DPL_P4 | !DPL_P3 to P0  Enable dynamic payload length for each pipe
"FEATURE   ",	// [1D]  = 0x00 0000 | 0 + !EN_DPL (dynamic payload disabled) + !EN_ACK_PAY (Enable payload with ACK) + !EN_DYN_ACK (Enables the W_TX_PAYLOAD_NOACK command)
  };

  U8  registerAddresses[] = {
0x00,
0x01,
0x02,
0x03,
0x04,
0x05,
0x06,
0x07,
0x08,
0x09,
//0x0A,
//0x0B,
0x0C,
0x0D,
0x0E,
0x0F,
//0x10,
0x11,
0x12,
0x13,
0x14,
0x15,
0x16,
0x17,
0x1C,
0x1D,
  };

  for ( U8 registerIndex=0; registerIndex < sizeof(registerAddresses); registerIndex++ ) {
    U8  value = Read( registerAddresses[registerIndex] );
    Serial.println( str( "Register %s (0x%02X) = 0x%02X", registerNames[registerIndex], registerAddresses[registerIndex], value ) );
  }

//  Serial.println( str( "NRF_STATUS = %02X", Read( NRF_STATUS ) ) );

  // Dump pipe addresses separately as they return up to 5 bytes
  U8  pipeAddress[5];
  Read( RX_ADDR_P0, 5, pipeAddress );
  Serial.print( "RX_ADDR_P0 = " );
  for ( int i=0; i < 5; i++ ) Serial.print( str( "%02X", pipeAddress[i] ) );
  Serial.println();

  Read( RX_ADDR_P1, 5, pipeAddress );
  Serial.print( "RX_ADDR_P1 = " );
  for ( int i=0; i < 5; i++ ) Serial.print( str( "%02X", pipeAddress[i] ) );
  Serial.println();

  Read( TX_ADDR, 5, pipeAddress );
  Serial.print( "TX_ADDR = " );
  for ( int i=0; i < 5; i++ ) Serial.print( str( "%02X", pipeAddress[i] ) );
  Serial.println();
}

// Reads from a register
U8  Read( U8 _address ) {
  Select();
  m_status = SPI.transfer( R_REGISTER | (_address & REGISTER_MASK) );
  U8  value = SPI.transfer( RF24_NOP );
  Deselect();
  return value;
}

// Read multiple bytes from a register
void  Read( U8 _address, U8 _count, U8* _value ) {
  Select();
  m_status = SPI.transfer( R_REGISTER | (_address & REGISTER_MASK) );
  while( _count-- > 0 ) {
    *_value++ = SPI.transfer( RF24_NOP );
  }
  Deselect();
}

// Reads a 32 bytes long payload
void  ReadPayload( U8& _payloadLength, U8* _payload ) {
  U8  payloadLength = 32; // Default length
  _payloadLength = payloadLength;

  Select();
  m_status = SPI.transfer( R_RX_PAYLOAD );
  while ( payloadLength-- > 0 ) {
    *_payload++ = SPI.transfer( RF24_NOP );
  }
  Deselect();
}

// Writes a command to a register
void  Write( U8 _command ) {
  Select();
  m_status = SPI.transfer( _command );
//Serial.println( str( "Status %02X", m_status ) );
  Deselect();
}

// Writes a value to a register
void  Write( U8 _address, U8 _value ) {
  Select();
  m_status = SPI.transfer( W_REGISTER | (_address & REGISTER_MASK) );
  SPI.transfer( _value );
//Serial.println( str( "Status %02X", m_status ) );
  Deselect();
}

// Writes multiple bytes to a register
void  Write( U8 _address, U8 _count, U8* _value ) {
  Select();
  m_status = SPI.transfer( W_REGISTER | (_address & REGISTER_MASK) );
//Serial.println( str( "Status %02X", m_status ) );
  while ( _count-- > 0 ) {
    SPI.transfer( *_value++ );
  }
  Deselect();
}

/*  Déjà écrit dans PushTXpacket
void  WritePayload( U8 _payloadLength, U8* _payload ) {
  Select();
  m_status = SPI.transfer( W_TX_PAYLOAD );
  while ( _payloadLength-- ) {
    SPI.transfer( *_payload++ );
  }
  Deselect();
}
*/

};

/*
// Appendix A => Configurer Shockburst Transmit (Appendix A page 65)
PRIM_RX = 0
TX_ADDR = target/receiving node address
  => Pas besoin de ré-écrire si pas changé entre transmits
TX_PLD = [1-32] bytes (bytes comptés par le chip, pas besoin de spécifier)
  => Ecrire séquentiellement pendant que CSN low
  => Si PTX doit recevoir ACK, data pipe 0 doit être configuré pour recevoir ACK
  => RX_ADDR_P0 = TX_ADDR
CE = High (min 10µs) => Starts transmission

  nRF24L01 fait son truc...

Si auto ack enabled (ENAA_P0=1) alors on passe en RX mode
  => Si packet reçu par receiver, TX_DS est mis (dans NRF_STATUS), payload removed du TX FIFO.
  => Si packet jamais reçu après plein de retransmits, IRQ MAX_RT doit être cleared (dans NRF_STATUS)

Si CE low
  => Standby-I (low current)
    => TX, RX, Power down
Sinon
  => le prochain payload dans la FIFO est transmis
    => Si FIFO vide
      => Standby-II
        => Si CE low
          => Standby-I
            => TX, RX, Power down


// Appendix A => Configurer Shockburst Receive (Appendix A page 65)
PRIM_RX = 1
EN_RXADDR = 1 sur le pipe
RX_PW_P? = mis à la bonne valeur
Régler les adresses comme dans item 2 

CE = high => Start RX mode
  => 130µs delay before communication stats

Si packet received
  => Stored in RX FIFO et RX_DR=1 (NFR_STATUS) et RX_P_NO = # data pipe où le payload est stocké (et d'où le payload provient)
  => Si auto ack enabled
    => On renvoie un ACK (sauf si le packet a spécifié NO_ACK) avec un payload optionnel

Si CE low
  => Standby-I (low current)
    => TX, RX, Power down
*/
