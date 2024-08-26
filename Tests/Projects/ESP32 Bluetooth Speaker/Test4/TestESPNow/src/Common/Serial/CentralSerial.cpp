#include "../Global.h"
#include "CentralSerial.h"
#include "driver/uart.h"

#ifdef NO_GLOBAL_SERIAL
Serial_Transmitter	Serial;
#endif

// Redefine error reports locally
void	CENTRAL_SERIAL_ERROR( bool _setError, const char* _functionName, const char* _message );
void	ESP_ERROR( esp_err_t _error, const char* _message );
//#undef ERROR
#undef ESP_ERROR_CHECK
//#define ERROR( _setError, _message ) CENTRAL_SERIAL_ERROR( _setError, __func__, _message )
#define ESP_ERROR_CHECK( x, message ) ESP_ERROR( x, message )

// Espressif doc page: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html?highlight=uart

// Default serial is 115200 8-N-1
// Tech Ref Manual pp 347:
//	• Programmable baud rate
//	• 1024 × 8-bit RAM shared by three UART transmit-FIFOs and receive-FIFOs
//	• Supports input baud rate self-check
//	• Supports 5/6/7/8 bits of data length
//	• Supports 1/1.5/2 STOP bits
//	• Supports parity bit
//	• Supports RS485 Protocol
//	• Supports IrDA Protocol
//	• Supports DMA to communicate data in high speed
//	• Supports UART wake-up
//	• Supports both software and hardware flow control

void	CentralSerial::Init( PORT _port, U32 _baudRate, U8 _pinTX, U8 _pinRX ) {
	m_port = _port;

	// Setup UART buffered IO with event queue
	const int 		uart_buffer_size = 2 * 1024;
	QueueHandle_t 	uart_queue;
	// Install UART driver using an event queue here
	ESP_ERROR_CHECK( uart_driver_install( m_port, uart_buffer_size, uart_buffer_size, 20, &uart_queue, 0 ), "Failed to install UART driver!" );	

//	const uart_port_t uart_num = UART_NUM_2;
	uart_config_t uart_config = {
		.baud_rate = int( _baudRate ),
		.data_bits = UART_DATA_8_BITS,	// 8
		.parity = UART_PARITY_DISABLE,	// N
		.stop_bits = UART_STOP_BITS_1,	// 1

//		.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,

		.rx_flow_ctrl_thresh = 122,
.source_clk = UART_SCLK_APB,  // ESP32, ESP32S2
	};

	// Configure UART parameters
	ESP_ERROR_CHECK( uart_param_config( m_port, &uart_config ), "Failed to configure UART!" );

//ESP_ERROR_CHECK( ESP_ERR_FLASH_NO_RESPONSE, "Testi!" );

//	uart_set_baudrate();
//	uart_set_word_length();	// selected out of uart_word_length_t Number of transmitted bits
//	uart_set_parity(); // selected out of uart_parity_t Parity control
//	uart_set_stop_bits(); // selected out of uart_stop_bits_t Number of stop bits
//	uart_set_hw_flow_ctrl(); // selected out of uart_hw_flowcontrol_t Hardware flow control mode
//	uart_set_mode(); // selected out of uart_mode_t Communication mode

	// Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19)
//	ESP_ERROR_CHECK( uart_set_pin( m_port, SOC_RX0, SOC_TX0, -1, -1 ), "Failed to set UART pins!" );
	ESP_ERROR_CHECK( uart_set_pin( m_port, _pinTX, _pinRX, -1, -1 ), "Failed to set UART pins!" );
//    if(uart_nr == 0) uartSetPins(0, SOC_RX0, SOC_TX0, -1, -1);

//ERROR( true, "Joe le Rigolo says Hello!" );
}

void	SerialSendPacketsTask( void* _param );
void	SerialReceivePacketsTask( void* _param );

void	CentralSerial::StartAutoSendTask( U8 _taskPriority, ISampleSource& _sampleSource ) {
	m_sampleSource = &_sampleSource;

	TaskHandle_t SerialSendPacketsTaskHandle;
	xTaskCreate( SerialSendPacketsTask, "SerialSendPacketsTask", 2048, this, _taskPriority, &SerialSendPacketsTaskHandle );
}

void	CentralSerial::StartAutoReceiveTask( U8 _taskPriority, TransportESPNow_Transmitter& _transmitter ) {
	m_transmitter = &_transmitter;

// Start an interrupt handler to handled UART RX events?

	TaskHandle_t SerialReceivePacketsTaskHandle;
	xTaskCreate( SerialReceivePacketsTask, "SerialReceivePacketsTask", 4096, this, _taskPriority, &SerialReceivePacketsTaskHandle );
}

void	CentralSerial::SendPacket( ISampleSource& _sampleSource ) {
	
//	// Write header
//	*((U16*) m_buffer) = m_header;
//
//	// Write packet info
//	_packetID <<= 8;				// Ignore MSB
//	_packetID |= _receiverMaskID;	// Replace by receiver mask ID
//	*((U32*) (m_buffer+2)) = _packetID;

	// Fill payload with samples
	U32	requestedSamplesCount = _sampleSource.GetChannelsCount() == ISampleSource::CHANNELS::STEREO ? SAMPLES_PER_PACKET : 2*SAMPLES_PER_PACKET;
	_sampleSource.GetSamples( _sampleSource.GetSamplingRate(), m_buffer, requestedSamplesCount );

	// Write packet
	int	writtenLength = uart_write_bytes( m_port, m_buffer, SAMPLES_PER_PACKET * sizeof(Sample) );
	ERROR( writtenLength != sizeof(m_buffer), "Couldn't write entire payload!" );
}

void	SerialSendPacketsTask( void* _param ) {
	CentralSerial*	that = (CentralSerial*) _param;

	const ITimeReference&	time = that->m_time;
	ISampleSource&			sampleSource = *that->m_sampleSource;

	U64	sendDeltaTime = (1000000ULL * (sampleSource.GetChannelsCount() == ISampleSource::STEREO ? CentralSerial::SAMPLES_PER_PACKET : 2*CentralSerial::SAMPLES_PER_PACKET))
					  / U64( sampleSource.GetSamplingRate() );	// How much time represents that many samples given our sampling rate?

	// Wait until time actually starts
	while ( !time.HasStarted() ) {
		delay( 1 );
	}

	U64	timeNextSend = time.GetTimeMicros() + sendDeltaTime;
	while ( true ) {
		vTaskDelay( 1 );

		// Should we send a packet?
		U64	now = time.GetTimeMicros();
		if ( now < timeNextSend )
			continue;	// Too soon!

// @TODO: Restore this! But it crashes the PC if these are not handled properly in time!
////		U32	packetID = that->m_sentPacketsCount;	// Use the transport's packet counter as packet ID, because the timer counter may have changed since it asked us to send the packet
//		that->SendPacket( sampleSource );

		timeNextSend += sendDeltaTime;
	}
}

void	CentralSerial::ProcessBlock2( const U8* _data, U32 _blockSize ) {
	while ( _blockSize > 0 ) {
		U32	bytesToEnd = ESP_NOW_MAX_DATA_LEN - m_packetOffset;
		if ( bytesToEnd > _blockSize ) {
			// Copy what we can
			memcpy( m_packet + m_packetOffset, _data, _blockSize );
			m_packetOffset += _blockSize;
			return;
		}

		// We can complete a packet!
		memcpy( m_packet + m_packetOffset, _data, bytesToEnd );
		m_transmitter->SendRawPacket( m_packet );
		m_packetOffset = 0;	// Restart a new packet...
		_blockSize -= bytesToEnd;
	}
}

// Processes a data block to assemble and send any detected radio packet that it can complete
// Radio packets must match header signature, packet length & checksum verification to be validated...
void	CentralSerial::ProcessBlock( const U8* _data, U32 _blockSize ) {
//Serial.printf( "ProcessBlock %d bytes\n", _blockSize );

	while ( _blockSize > 0 ) {
		if ( m_packetOffset < 6 ) {
			// Process header
			U8	d = *_data++; _blockSize--;
			if ( m_packetOffset == 0 ) {
				if ( d != 0x34 && d != 0x35 ) {
					continue;	// Not the expected header bytes!
				}
			} else if ( m_packetOffset == 1 ) {
				if ( d != 0x12 ) {
//Serial.println( "No 0x12" );
					ResetPacket();	// Invalidate packet!
					continue;
				}
			} else if ( m_packetOffset == 3 ) {
				if ( d != ESP_NOW_MAX_DATA_LEN ) {	// Check packet length is 250 bytes
//Serial.println( "No packet length = 250" );
					ResetPacket();	// Invalidate packet!
					continue;
				}
			}

			// Copy byte...
			m_packet[m_packetOffset++] = d;
			continue;
		}

		// Copy payload
		U32	bytesToEnd = ESP_NOW_MAX_DATA_LEN - m_packetOffset;
		U32	copyLength = min( bytesToEnd, _blockSize );
		memcpy( m_packet + m_packetOffset, _data, copyLength );
		m_packetOffset += copyLength;
		_blockSize -= copyLength;

		if ( m_packetOffset != ESP_NOW_MAX_DATA_LEN )
			continue;	// The rest of the payload will be available later...

		// Packet is complete!
		// Compute checksum for final validation...
		U16		receivedChecksum = *((U16*) (m_packet + 4));
		U16*	checkSumValues = (U16*) (m_packet + 6);
		U16		checkSum = 0;
		U16		checkSumValue;
		bool	saw1234 = false;
		for ( U32 i=0; i < 122; i++ ) {
			checkSumValue = *checkSumValues++;
			checkSum += checkSumValue;
			saw1234 |= checkSumValue == 0x1234;
		}
//Serial.printf( "Checksum = %04X vs. received %04X (next bytes %08X)\n", m_packetChecksum, receivedChecksum, _blockSize >= 4 ? *((U32*) _data) : 0xAAAAAAAAU );

//for ( U32 i=0; i < 15; i++ ) {
//	Serial.printf( "%04X %04X %04X %04X %04X %04X %04X %04X\n", ((U16*) m_packet)[8*i+0], ((U16*) m_packet)[8*i+1], ((U16*) m_packet)[8*i+2], ((U16*) m_packet)[8*i+3], ((U16*) m_packet)[8*i+4], ((U16*) m_packet)[8*i+5], ((U16*) m_packet)[8*i+6], ((U16*) m_packet)[8*i+7] );
//}
//Serial.printf( "%04X %04X %04X %04X %04X\n\n", ((U16*) m_packet)[8*15+0], ((U16*) m_packet)[8*15+1], ((U16*) m_packet)[8*15+2], ((U16*) m_packet)[8*15+3], ((U16*) m_packet)[8*15+4] );
//Serial.printf( "Checksum = %04X vs. received %04X - Diff = %04X (next bytes %08X)\n", checkSum, receivedChecksum, checkSum - receivedChecksum, *((U32*) _data) );

		if ( receivedChecksum == checkSum ) {
			// Validated! Send the packet!
			U32&	packetID = *((U32*) (m_packet + 2));
					packetID = (packetID & 0xFF000000UL) | (m_packetIDAudio++ & 0x00FFFFFFUL);	// Keep receiver mask ID but replace packet length & checksum by actual audio packet ID

// @TODO: Handle command packets!

			m_transmitter->SendRawPacket( m_packet );
		} else {
//Serial.printf( "Invalid checksum!%s\n", saw1234 ? " But saw 1234!" : "" );
		}

		ResetPacket();
	}

#if 0
	*((U16*) m_packet) = 0x1234;
	if ( length != ESP_NOW_MAX_DATA_LEN ) {
//		Serial.printf( "Received unexpected amount of data (%d bytes instead of %d)", length, ESP_NOW_MAX_DATA_LEN );
//		uart_flush( port );

		while ( length != 0 ) {
			int	L = min( 512, length );
			uart_read_bytes( port, data, L, 100 );
Check if we got a valid packet in there, then synchronize...
		}

		continue;
	}
	int		actualLength = uart_read_bytes( port, data, length, 100 );
	ERROR( actualLength != length, "Lengths mismatch!" );

#if 1	// Receive full packet
	memcpy( m_packet, data, ESP_NOW_MAX_DATA_LEN );
#else	// Assemble packet payload only
	// Decode header + receiverMaskID
	U8	receiverMaskID = 0xFF;	// Broadcast to all devices by default...

	// Forward payload to transmitter
	U32	packetID = 1;
	packetID <<= 8;				// Ignore MSB
	packetID |= receiverMaskID;	// Replace by receiver mask ID
	*((U32*) (m_packet+2)) = packetID;

#endif

	transmitter.SendRawPacket( m_packet );

//		U32	packetID = that->m_sentPacketsCount;	// Use the transport's packet counter as packet ID, because the timer counter may have changed since it asked us to send the packet
//		that->SendPacket( sampleSource );
#endif
}

// Reset the packet assembler's state machine and packet offset
void	CentralSerial::ResetPacket() {
//	m_processState = PS_AWAITING_HEADER_BYTE0;
	m_packetOffset = 0;
}

void	SerialReceivePacketsTask( void* _param ) {
	CentralSerial&	that = *((CentralSerial*) _param);

	const ITimeReference&	time = that.m_time;
	CentralSerial::PORT		port = that.m_port;

//	U64	sendDeltaTime = (1000000ULL * (sampleSource.GetChannelsCount() == ISampleSource::STEREO ? CentralSerial::SAMPLES_PER_PACKET : 2*CentralSerial::SAMPLES_PER_PACKET))
//					  / U64( sampleSource.GetSamplingRate() );	// How much time represents that many samples given our sampling rate?
//

	// Wait until time actually starts
	while ( !time.HasStarted() ) {
		delay( 1 );
	}

	const U32	DATA_SIZE = 512;
	U8	data[DATA_SIZE + 4];

	while ( true ) {
		vTaskDelay( 1 );

		// Check if there's anything available on UART
		U32	length = 0;
		ESP_ERROR_CHECK( uart_get_buffered_data_len( port, (size_t*) &length ), "Failed to read serial data from UART!" );
		if ( length == 0 )
			continue;

		// Process entire blocks
		while ( length > 0 ) {
			U32	blockSize = min( DATA_SIZE, length );
			U32	readSize = uart_read_bytes( port, data, blockSize, 100 );
			ERROR( readSize != blockSize, "Couldn't read the required block size!" );
			length -= blockSize;
//			that.ProcessBlock( data, blockSize );
			that.ProcessBlock2( data, blockSize );
		}
	}
}

//void	CentralSerial::Read() {
//	// Read data from UART.
////	const uart_port_t uart_num = UART_NUM_2;
//	uint8_t	data[128];
//	int		length = 0;
//	ESP_ERROR_CHECK( uart_get_buffered_data_len( m_port, (size_t*) &length ), "Failed to read serial data from UART!" );
//	length = uart_read_bytes( m_port, data, length, 100 );
//}
//
//void	CentralSerial::Write() {
//	// Write data to UART.
//	const char*	test_str = "This is a test string.\r\n";
//	int			length =  strlen( test_str );
//	int	writtenLength = uart_write_bytes( m_port, (const char*) test_str, length );
//	ERROR( writtenLength != length, "Lengths mismatch!" );
//}

// Starts an actual serial to output debug lines after an error
void	CENTRAL_SERIAL_ERROR( bool _setError, const char* _functionName, const char* _message ) {
	if ( !_setError ) return;

	// Open an actual debug serial to continually output some messages
	HardwareSerial	serial(0);
	serial.begin( 115200 );
	while ( true ) {
		digitalWrite( PIN_LED_RED, _setError ); _setError = !_setError;
		serial.print( "ERROR " );
		serial.print( _functionName );
		serial.print( "() => " );
		serial.println( _message );
		delay( 1000 );
	}
}

void	ESP_ERROR( esp_err_t _error, const char* _message ) {
	if ( _error == ESP_OK )
		return;
	char	message[512];
	ERROR( true, str( "%s (%s)", _message, esp_err_to_name_r( _error, message, 512 ) ) );
}
