#include "../Global.h"
#include "TransportESPNow.h"

#include <esp_now.h>
#include <esp_wifi.h>

const U8	broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


TransportESPNow_Base::TransportESPNow_Base() {
}
TransportESPNow_Base::~TransportESPNow_Base() {
}

void	TransportESPNow_Base::Init( U8 _WiFiChannel ) {
	// Set Wifi channel
	esp_wifi_set_promiscuous( true );
	esp_wifi_set_channel( _WiFiChannel, WIFI_SECOND_CHAN_NONE );
	esp_wifi_set_promiscuous( false );


// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_now.html#group__ESPNOW__APIs_1gaa2774e8840ff2907db46f9444a8a5728
//esp_err_t esp_now_set_peer_rate_config( const uint8_t* peer_addr, esp_now_rate_config_t* config);
//esp_now_rate_config_t
//wifi_phy_mode_t
//esp_now_rate_config

// ERSU info (from https://github.com/espressif/esp-idf/issues/12216)
//hi, @MacWyznawca , ERSU is a transmission mode related to 802.11 ax . another mode in 802.11 ax is SU. this setting means sending ESP-NOW frame in ERSU mode. ERSU is always usde in long range transmission, and its frame has lower rate compared with SU mode

// From https://esp32.com/viewtopic.php?t=12781
//if you want to use esp_wifi_internal_set_fix_rate, please disable WiFi AMPDU TX by:
//make menuconfig => components => Wi-Fi => Disable TX AMPDU.

// Interesting thread: https://www.esp32.com/viewtopic.php?t=9965

	esp_err_t	result = esp_now_init();
	ERROR( result != ESP_OK, str( "ESPNow Init failed: %s", esp_err_to_name(result) ) );
//Serial.println( "ESPNow Init Success" );

	// This will broadcast a message to everyone in range
	esp_now_peer_info_t	peerInfo = {};
	memcpy( &peerInfo.peer_addr, broadcastAddress, 6 );
	if ( !esp_now_is_peer_exist( broadcastAddress ) ) {
		Serial.println( "Broadcast peer doesn't exist, adding peer..." );
		result = esp_now_add_peer( &peerInfo );
		ERROR( result != ESP_OK, str( "Failed to add broadcast peer: %s", esp_err_to_name(result) ) );
	}
	
	// Setup bandwidth
	result = esp_now_get_peer( peerInfo.peer_addr, &peerInfo );
	ERROR( result != ESP_OK, str( "Failed to get broadcast peer: %s", esp_err_to_name(result) ) );
//Serial.println( str( "WiFi Interface = %s", peerInfo.ifidx == wifi_interface_t::WIFI_IF_AP ? "WIFI_IF_AP" : "WIFI_IF_STA" ) );

//	result = esp_wifi_config_espnow_rate( peerInfo.ifidx, wifi_phy_rate_t::WIFI_PHY_RATE_2M_L );	// 2 Mbps with long preamble <= Lots of ESPNOW_NO_MEM errors!
	result = esp_wifi_config_espnow_rate( peerInfo.ifidx, wifi_phy_rate_t::WIFI_PHY_RATE_5M_S );	// 5.5 Mbps with short preamble <= Lot less of ESPNOW_NO_MEM errors but they sometimes happens in bursts...
//	result = esp_wifi_config_espnow_rate( peerInfo.ifidx, wifi_phy_rate_t::WIFI_PHY_RATE_5M_L );	// 5.5 Mbps with long preamble <= Less packets lost! (~60 packets over 723)
//	result = esp_wifi_config_espnow_rate( peerInfo.ifidx, wifi_phy_rate_t::WIFI_PHY_RATE_11M_L );	// 11 Mbps with long preamble <= Can't really see the difference? :/
	ERROR( result != ESP_OK, str( "Failed to setup WiFi broadcast rate: %s", esp_err_to_name(result) ) );
	Serial.println( "Successfully set WiFi rate to 5.5 Mbps" );

	Serial.println( "TransportESPNow Initialized!" );
}



///////////////////////////////////////////////////////////////////////////
// Receiver
///////////////////////////////////////////////////////////////////////////
//
static TransportESPNow_Receiver*	gs_instance = NULL;	// Annoyingly we can't pass a param to the callback function so we need a singleton

void	ReceiveCallback( const U8* _MACAddress, const U8* _data, int _dataLength );

TransportESPNow_Receiver::TransportESPNow_Receiver() : TransportESPNow_Base() {
	ERROR( gs_instance != NULL, "Singleton already exists!" );
	gs_instance = this;

/*
	m_bufferReceiveSize = _bufferReceiveSize;
	m_bufferReceive = new U8[m_bufferReceiveSize];
	ERROR( m_bufferReceive == NULL, "Failed to allocate m_bufferReceive!" );
*/
}
TransportESPNow_Receiver::~TransportESPNow_Receiver() {
//	delete[] m_bufferSend;
	delete[] m_buffer;
}

void	TransportESPNow_Receiver::Init( U8 _WiFiChannel, U8 _receiverMaskID, U32 _bufferSize ) {
	TransportESPNow_Base::Init( _WiFiChannel );

	m_receiverMaskID = _receiverMaskID;

	delete[] m_buffer;

	// Create the circular samples buffer
	m_bufferSize = _bufferSize;
	m_buffer = new Sample[m_bufferSize];
	memset( m_buffer, 0, m_bufferSize * sizeof(Sample) );
	m_sampleIndexWrite = 0;
	m_sampleIndexRead = 0;

	// Start listening for audio packets...
	esp_now_register_recv_cb( ReceiveCallback );
}

void	TransportESPNow_Receiver::SetHeader( U32 _headerSize, const U8* _header ) {
	ERROR( _headerSize >= ESP_NOW_MAX_DATA_LEN, "Header size larger than buffer!" );
	m_headerSize = _headerSize;
	m_header = new U8[m_headerSize];
	memcpy( m_header, _header, m_headerSize );
}

void	TransportESPNow_Receiver::GetSamples( Sample* _samples, U32 _samplesCount ) {
	ERROR( _samplesCount > m_bufferSize, str( "Buffer size is too small (%d samples), can't provide %d samples to client!", m_bufferSize, _samplesCount ) );


/* OK! Clear 1KHz sound!
static U32	s_sampleIndex = 0;
for ( U32 i=0; i < _samplesCount; i++, _samples++ ) {
	S32	temp = S16( 32767 * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );
	_samples->left = _samples->right = temp;
}
return;
*/

/* OK! Clear 1KHz sound as well => Copy from buffer working...
static U32	s_sampleIndex = 0;
for ( U32 i=0; i < _samplesCount; i++ ) {
	S32	temp = S16( 32767 * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );
	m_buffer[(m_sampleIndexWrite + m_bufferSize - _samplesCount + i) % m_bufferSize].left = temp;
	m_buffer[(m_sampleIndexWrite + m_bufferSize - _samplesCount + i) % m_bufferSize].right = temp;
}
*/


	U32	sampleIndex = m_sampleIndexRead % m_bufferSize;	// Constrain within buffer
	U32	samplesCountToEnd = min( m_bufferSize - sampleIndex, _samplesCount );
	memcpy( _samples, m_buffer + sampleIndex, samplesCountToEnd * sizeof(Sample) );
	_samples += samplesCountToEnd;
	_samplesCount -= samplesCountToEnd;
	m_sampleIndexRead += samplesCountToEnd;

	if ( _samplesCount > 0 ) {
		// Wrap around buffer and copy the rest
		memcpy( _samples, m_buffer, _samplesCount * sizeof(Sample) );
		m_sampleIndexRead += _samplesCount;
	}
}

void	TransportESPNow_Receiver::Receive( const U8* _senderMACAddress, const U8* _payload, U32 _payloadSize ) {
	// Check we received the expected header
	if ( _payloadSize < m_headerSize+2 ) {
		m_lastReceivedPacketStatus = INVALID_HEADER;
		return;	// Not long enough
	}
	if ( memcmp( _payload, m_header, m_headerSize ) ) {
		m_lastReceivedPacketStatus = INVALID_HEADER;
		return;	// Not the expected header...
	}

	_payload += m_headerSize;	// Skip header

	// Read receiver mask ID and check if this payload is addressed to us
	U8	receiverMaskID = *_payload++;
	if ( (receiverMaskID & m_receiverMaskID) == 0 ) {
		m_lastReceivedPacketStatus = NOT_FOR_US;
		return;
	}

	// Read packet ID & check for lost packets
	U8	packetID = *_payload++;
	U8	lostPacketsCount = packetID - m_lastReceivedPacketID - 1;
//	if ( (packetID == 0 && m_lastReceivedPacketID != 0xFF)
//	  || (packetID != m_lastReceivedPacketID+1) ) {
	if ( lostPacketsCount != 0 ) {
		// Write 0 in place of lost packets
// Actually, this makes things worse! :D
// Should we duplicate last packets?
//		for ( U32 sampleIndex=SAMPLES_PER_PACKET*lostPacketsCount; sampleIndex > 0; sampleIndex--, m_sampleIndexWrite++ ) {
//			memset( m_buffer + (m_sampleIndexWrite % m_bufferSize), 0, sizeof(Sample) );
//		}

// Sound is worse if we skip packets! :/
//		m_sampleIndexWrite += lostPacketsCount * SAMPLES_PER_PACKET;

		m_lostPacketsCount += lostPacketsCount;
		m_receivedPacketsCount += lostPacketsCount;
//Serial.printf( "Lost packet %02X / %02X = %d\n", m_lastReceivedPacketID, packetID, lostPacketsCount );
	}
//Serial.printf( "Packet ID %02X / %02X\n", m_lastReceivedPacketID, packetID );

	m_lastReceivedPacketID = packetID;

	// Read samples
	_payloadSize -= m_headerSize + 2;
	if ( _payloadSize != SAMPLES_PER_PACKET*sizeof(Sample) ) {
		m_lastReceivedPacketStatus = INVALID_PAYLOAD_SIZE;
Serial.println( str( "Received unexpected payload size %d", _payloadSize ) );
		return;
	}

//static U32	s_sampleIndex = 0;
	for ( U32 sampleIndex=0; sampleIndex < SAMPLES_PER_PACKET; sampleIndex++, m_sampleIndexWrite++ ) {

//((Sample*) _payload)->left = S16( 32767 * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );
//((Sample*) _payload)->right = ((Sample*) _payload)->left;

		memcpy( m_buffer + (m_sampleIndexWrite % m_bufferSize), _payload, sizeof(Sample) );
		_payload += sizeof(Sample);
	}

//Serial.println( "Received!" );
	m_receivedPacketsCount++;
}

void	ReceiveCallback( const U8* _senderMACAddress, const U8* _data, int _dataLength ) {
	gs_instance->Receive( _senderMACAddress, _data, _dataLength );
}


///////////////////////////////////////////////////////////////////////////
// Transmitter
///////////////////////////////////////////////////////////////////////////
//
TransportESPNow_Transmitter::TransportESPNow_Transmitter() : TransportESPNow_Base() {
	m_buffer = new U8[ESP_NOW_MAX_DATA_LEN];
}
TransportESPNow_Transmitter::~TransportESPNow_Transmitter() {
	delete[] m_buffer;
}

void	TransportESPNow_Transmitter::Init( U8 _WiFiChannel ) {
	TransportESPNow_Base::Init( _WiFiChannel );
}

void	TransportESPNow_Transmitter::SetHeader( U32 _headerSize, const U8* _header ) {
	ERROR( _headerSize >= ESP_NOW_MAX_DATA_LEN, "Header size larger than buffer!" );
	m_headerSize = _headerSize;
	memcpy( m_buffer, _header, m_headerSize );
}

/*void	TransportESPNow_Transmitter::WriteSample( const Sample& _sample ) {
	m_buffer[m_headerSize + m_payloadSize] = (_sample.left + 32768) >> 8;
	m_payloadSize++;

	// Send if we completed a packet
	if ( (m_headerSize + m_payloadSize) == ESP_NOW_MAX_DATA_LEN ) {
		Send();
	}
}

void	TransportESPNow_Transmitter::WriteSamples( ISampleSource& _sampleSource, U32 _samplesCount ) {

static Sample	s_tempSamples[ESP_NOW_MAX_DATA_LEN];
static U32		s_sampleIndex = 0;

	U32	availableSamplesCount = ESP_NOW_MAX_DATA_LEN - m_headerSize - m_payloadSize;
	while ( _samplesCount > 0 ) {
		// Read some samples from the source
		U32	samplesCount = min( availableSamplesCount, _samplesCount );
		_sampleSource.GetSamples( s_tempSamples, samplesCount );
		m_payloadSize += samplesCount;

// Process samples because we can't send 16-bits stereo samples directly at the present time
Sample*	sample = s_tempSamples;
for ( U32 sampleIndex=0; sampleIndex < samplesCount; sampleIndex++, sample++ ) {


// Simulate a perfect sine wave
//sample->left = S16( 32767 * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );


	m_buffer[m_headerSize+sampleIndex] = U8( (sample->left / 256 + 128) );
}

		Send();
		_samplesCount -= samplesCount;
		availableSamplesCount = ESP_NOW_MAX_DATA_LEN - m_headerSize;	// m_sampleIndex has been reset to 0 by Send(), we now have the entire buffer available...
	}
}
*/
void	TransportESPNow_Transmitter::SendPacket( ISampleSource& _sampleSource, U8 _packetID, U8 _receiverMaskID ) {
	// Write packet info
	m_buffer[m_headerSize] = _receiverMaskID;
	m_buffer[m_headerSize+1] = _packetID;

	// We have 250 - 4 = 246 bytes available for payload, each sample is 4 bytes, so we can only ask for SAMPLES_PER_PACKET samples at a time
	_sampleSource.GetSamples( (Sample*) (m_buffer + m_headerSize + 2), SAMPLES_PER_PACKET );
	m_payloadSize = 2 + SAMPLES_PER_PACKET * sizeof(Sample);


/*// Simulate a 1KHz sine wave
static U32		s_sampleIndex = 0;
Sample*	sample = (Sample*) (m_buffer + m_headerSize + 2);
for ( U32 i=0; i < SAMPLES_PER_PACKET; i++, sample++ ) {
	S32	temp = S16( 32767 * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );
	sample->left = temp;
	sample->right = temp;
}
//*/


	Send();
}


void	TransportESPNow_Transmitter::Flush() {
	if ( m_payloadSize > 0 ) {
		Send();
	}
}

void	TransportESPNow_Transmitter::Send() {
	esp_err_t	result = esp_now_send( broadcastAddress, m_buffer, m_headerSize + m_payloadSize );
	if ( result != ESP_OK ) {
		Serial.printf( "Failed to send: %s\n", esp_err_to_name(result) );
//m_sampleIndex = 0;
//return;
	}

//Serial.printf( "Sent %d bytes", m_headerSize + m_sampleIndex );
	
    m_payloadSize = 0;

// Monitor packets...
static U32	packetsCounter = 0;
packetsCounter++;
digitalWrite( PIN_LED_RED, (packetsCounter & 0x80) != 0 );
}
