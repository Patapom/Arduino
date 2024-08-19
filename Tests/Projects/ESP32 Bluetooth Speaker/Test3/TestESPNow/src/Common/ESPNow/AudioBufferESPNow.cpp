#include "../Global.h"
#include "AudioBufferESPNow.h"

#if 0

#include <esp_now.h>
#include <esp_wifi.h>

const U8	broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static AudioBufferESPNow*	gs_instance = NULL;	// Annoyingly we can't pass a param to the callback function so we need a singleton

void	ReceiveCallback( const U8* _MACAddress, const U8* _data, int _dataLength );

AudioBufferESPNow::AudioBufferESPNow( U32 _bufferReceiveSize, U32 _bufferSendSize ) {
	ERROR( gs_instance != NULL, "Singleton already exists!" );
	gs_instance = this;

	ERROR( _bufferSendSize > ESP_NOW_MAX_DATA_LEN, "_bufferSendSize xeceeds max ESP-Now packet size!" );
	m_bufferSendSize = _bufferSendSize;
	m_bufferSend = new U8[m_bufferSendSize];
	ERROR( m_bufferSend == NULL, "Failed to allocate m_bufferSend!" );

	m_bufferReceiveSize = _bufferReceiveSize;
	m_bufferReceive = new U8[m_bufferReceiveSize];
	ERROR( m_bufferReceive == NULL, "Failed to allocate m_bufferReceive!" );
}
AudioBufferESPNow::~AudioBufferESPNow() {
	delete[] m_bufferSend;
	delete[] m_bufferReceive;
}

void	AudioBufferESPNow::Init( U8 _WiFiChannel ) {
	// Set Wifi channel
	esp_wifi_set_promiscuous( true );
	esp_wifi_set_channel( _WiFiChannel, WIFI_SECOND_CHAN_NONE );
	esp_wifi_set_promiscuous( false );
	
// Test this!
//esp_now_peer_info_t	peerInfo;
//esp_err_t esp_now_get_peer(const uint8_t *peer_addr, esp_now_peer_info_t *peer);
//esp_wifi_config_espnow_rate( peerInfo.ifidx, wifi_phy_rate_t::WIFI_PHY_RATE_5M_S );	// 5.5 Mbps with short preamble


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
	esp_now_register_recv_cb( ReceiveCallback );

	// This will broadcast a message to everyone in range
	esp_now_peer_info_t	peerInfo = {};
	memcpy( &peerInfo.peer_addr, broadcastAddress, 6 );
	if ( !esp_now_is_peer_exist( broadcastAddress ) ) {
		result = esp_now_add_peer( &peerInfo );
		ERROR( result != ESP_OK, str( "Failed to add broadcast peer: %s", esp_err_to_name(result) ) );
	}

	Serial.println( "AudioBufferESPNow Initialized!" );
}

void	AudioBufferESPNow::AddSample( const Sample& _sample ) {
	m_bufferSend[m_headerSize + m_sampleSendIndex] = (_sample.left + 32768) >> 8;
	m_sampleSendIndex++;

	// Send if we completed a packet
	if ( (m_headerSize + m_sampleSendIndex) == m_bufferSendSize ) {
		Send();
	}
}

void	AudioBufferESPNow::Flush() {
	if ( m_sampleSendIndex > 0 ) {
		Send();
	}
}

void	AudioBufferESPNow::SetHeader( U32 _headerSize, const U8* _header ) {
	ERROR( _headerSize >= m_bufferSendSize, "Header size larger than buffer!" );

	m_headerSize = _headerSize;
	memcpy( m_bufferSend, _header, m_headerSize );
}

void	AudioBufferESPNow::GetSamples( Sample* _samples, U32  _samplesCount ) {
	Sample*	sample = _samples;
	for ( int sampleIndex=0; sampleIndex < _samplesCount; sampleIndex++, sampleIndex++ ) {
		// At the moment, we're only transporting 8-bits samples
		U8	sample8Bits = m_bufferReceive[m_playerSampleIndex];
		S16	left = (S16(sample8Bits) - 128) * 255;


		sample->left = left;
		sample->right = left;

		m_playerSampleIndex++;
		if ( m_playerSampleIndex == m_bufferReceiveSize ) {
			m_playerSampleIndex = 0;	// Loop...
		}
	}
}

void	AudioBufferESPNow::Send() {
	esp_err_t	result = esp_now_send( broadcastAddress, m_bufferSend, m_headerSize + m_sampleSendIndex );
	if ( result != ESP_OK ) {
		Serial.printf( "Failed to send: %s\n", esp_err_to_name(result) );
	}
    m_sampleSendIndex = 0;

// Monitor packets...
static U32	packetsCounter = 0;
packetsCounter++;
digitalWrite( PIN_LED_RED, (packetsCounter & 0x80) != 0 );
}

void	AudioBufferESPNow::Receive( const U8* _senderMACAddress, const U8* _payload, U32 _payloadSize ) {
	for ( U32 sampleIndex=0; sampleIndex < _payloadSize; sampleIndex++ ) {
		m_bufferReceive[m_sampleReceiveIndex] = *_payload++;
		m_sampleReceiveIndex++;
		if ( m_sampleReceiveIndex > m_bufferReceiveSize )
			m_sampleReceiveIndex = 0;	// Loop around...
	}
}

void	ReceiveCallback( const U8* _senderMACAddress, const U8* _data, int _dataLength ) {
	// Check we received the expected header
	if ( _dataLength < gs_instance->m_headerSize )
		return;	// Not long enough
	if ( memcmp( _data, gs_instance->m_bufferSend, gs_instance->m_headerSize ) )
		return;	// Not the expected header...

	U32	payloadSize = U32(_dataLength) - gs_instance->m_headerSize;
	gs_instance->Receive( _senderMACAddress, _data + gs_instance->m_headerSize, payloadSize );
}

#endif
