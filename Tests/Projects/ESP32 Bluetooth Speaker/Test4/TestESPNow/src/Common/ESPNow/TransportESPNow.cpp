#include "../Global.h"
#include "TransportESPNow.h"

#include <esp_now.h>
#include <esp_wifi.h>

const U8	broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

//#define SIMULATE_WAVE_FORM_RECEIVER	1024	// Should sound like a clear 1KHz sound if all packets were received correctly...
//#define SIMULATE_WAVE_FORM_TRANSMITTER	1024	// Should sound like a clear 1KHz sound if all packets were received correctly...

#if defined(SIMULATE_WAVE_FORM_RECEIVER) || defined(SIMULATE_WAVE_FORM_TRANSMITTER)
static U32	s_sampleIndex = 0;
#endif


//@TODO: Check WiFi home channel and avoid?
// Cf. => https://www.electrosoftcloud.com/en/esp32-wifi-and-esp-now-simultaneously/
//
void	TransportESPNow_Base::Init( U8 _WiFiChannel, U32 _samplingRate ) {
	m_samplingRate = _samplingRate;

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

	Serial.printf( "TransportESPNow Initialized for %d Hz!", m_samplingRate );
}


///////////////////////////////////////////////////////////////////////////
// Receiver
///////////////////////////////////////////////////////////////////////////
//
static TransportESPNow_Receiver*	gs_instance = NULL;	// Annoyingly we can't pass a param to the callback function so we need a singleton

void	ReceiveCallback( const U8* _MACAddress, const U8* _data, int _dataLength );

TransportESPNow_Receiver::TransportESPNow_Receiver( const ITimeReference& _time )
	 : TransportESPNow_Base(), AudioBuffer( _time ) {
	ERROR( gs_instance != NULL, "Singleton already exists!" );
	gs_instance = this;
}
TransportESPNow_Receiver::~TransportESPNow_Receiver() {
}

bool	TransportESPNow_Receiver::Init( U8 _WiFiChannel, U8 _receiverMaskID, U32 _samplingRate, CHANNELS _channelsCount, float _preLoadDelay ) {
	TransportESPNow_Base::Init( _WiFiChannel, _samplingRate );

	m_receiverMaskID = _receiverMaskID;
	m_channelsCount = _channelsCount;

	// Allocate the audio buffer
	U32	preLoadSamplesCount = U32( ceil( _preLoadDelay * m_samplingRate ) );
	U32	bufferSize = preLoadSamplesCount + 8 * SAMPLES_PER_PACKET;	// Give room for 8 updates before we loop back into "read territory"

 	if ( !AudioBuffer::Init( bufferSize, preLoadSamplesCount, _preLoadDelay * 1000000.0f ) )
		return false;

	// Start listening for audio packets...
	esp_now_register_recv_cb( ReceiveCallback );

	return true;
}

void	TransportESPNow_Receiver::Receive( const U8* _senderMACAddress, const U8* _payload, U32 _payloadSize ) {
	if ( !m_time->HasStarted() )
		return;	// Wait until the time reference starts!

	// Check we received the expected header
	if ( _payloadSize < ESP_NOW_MAX_DATA_LEN ) {
		m_lastReceivedPacketStatus = INVALID_PAYLOAD_SIZE;
Serial.printf( "Received unexpected payload size %d\n", _payloadSize );
		return;	// Not long enough
	}
	if ( memcmp( _payload, &m_header, sizeof(U16) ) ) {
		m_lastReceivedPacketStatus = INVALID_HEADER;
Serial.printf( "Unexpected header 0x%04X\n", *((U16*) _payload) );
		return;	// Not the expected header...
	}

	_payload += sizeof(U16);	// Skip header

	// Read receiver mask ID and packet ID
	U32	packetID = *((U32*) _payload);
	_payload += sizeof(U32);

	// Check if this payload is addressed to us
	U8	receiverMaskID = U8( packetID );
	if ( (receiverMaskID & m_receiverMaskID) == 0	// Just a regular mask?
	 && receiverMaskID != m_receiverMaskID ) {		// Or is it addressed to the Central (ID 0) and we are it?
		m_lastReceivedPacketStatus = NOT_FOR_US;
Serial.println( "Not for us!" );
		return;
	}

	// Read packet ID & check for lost packets
	packetID >>= 8;
	packetID |= m_lastReceivedPacketID & 0xFF000000UL;	// Always re-use last packet's MSB

	// Fix ID looping after 2^24 packets:
	//  • Our last received packet ID is on 32 bits and could be 0xHH^^^^^^ where HH € [0,255] and 0x00^^^^^^ is a very high 24-bits number (close to looping)
	//	• Our new packet ID is suddendly jumping back down to a very low number like 0xHHvvvvvv < 0xHH^^^^^^
	if ( packetID < m_lastReceivedPacketID ) {
		packetID += 0x01000000UL;	// To fix this, we simply add one to the MSB of the packetID...
	}

	U32	lostPacketsCount = packetID - m_lastReceivedPacketID - 1;
	if ( lostPacketsCount != 0 ) {
		if ( lostPacketsCount < 10 ) {

// @TODO: Restore lost packets??

		}
		m_lostPacketsCount += lostPacketsCount;
		m_receivedPacketsCount += lostPacketsCount;
//Serial.printf( "Lost packet %02X / %02X = %d\n", m_lastReceivedPacketID, packetID, lostPacketsCount );
	}
//Serial.printf( "Packet ID %02X / %02X\n", m_lastReceivedPacketID, packetID );


#ifdef SIMULATE_WAVE_FORM_RECEIVER
for ( U32 i=0; i < SAMPLES_PER_PACKET; i++ ) {
//	S32	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );
S32	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / 22050.0f) * s_sampleIndex++ ) );
	((Sample*) _payload)[i].left = temp;
	((Sample*) _payload)[i].right = temp;
}
#endif

	// Write samples
	WriteSamples( (Sample*) _payload, SAMPLES_PER_PACKET );

	m_receivedPacketsCount++;
	U32	oldLastReceivedPacketID = m_lastReceivedPacketID;
	m_lastReceivedPacketID = packetID;

	// Notify
	if ( m_onPacketsReceivedCallback != NULL ) {
		(*m_onPacketsReceivedCallback)( oldLastReceivedPacketID );
	}
}

#if 0
U32	TransportESPNow_Receiver::GetSamples( Sample* _samples, U32 _samplesCount ) {
	ERROR( _samplesCount > m_bufferSize, str( "Buffer size is too small (%d samples), can't provide %d samples to client!", m_bufferSize, _samplesCount ) );

#if 0	// => Works perfectly with I2S task requesting samples!
static U32	s_sampleIndex = 0;
for ( U32 i=0; i < _samplesCount; i++ ) {
	S16	value = S16( 32768 * sin( 2*PI * (1000.0 / 22050) * s_sampleIndex++ ) );
	_samples[i].left = value;
	_samples[i].right = value;
}
return _samplesCount;
#endif

	_samplesCount = min( m_sampleIndexWrite - m_sampleIndexRead, _samplesCount );	// Constrain to what is available in the buffer now...
	U32	bufferSampleIndex = m_sampleIndexRead % m_bufferSize;						// Constrain source index within buffer
	U32	samplesCountToEnd = min( m_bufferSize - bufferSampleIndex, _samplesCount );
	if ( _samplesCount <= samplesCountToEnd ) {	// Transfer in a single shot
		memcpy( _samples, m_buffer + bufferSampleIndex, _samplesCount * sizeof(Sample) );
	} else {	// Split in 2
		memcpy( _samples, m_buffer + bufferSampleIndex, samplesCountToEnd * sizeof(Sample) );
		memcpy( _samples + samplesCountToEnd, m_buffer, (_samplesCount - samplesCountToEnd) * sizeof(Sample) );
	}
	m_sampleIndexRead += _samplesCount;

	return _samplesCount;
}

void	TransportESPNow_Receiver::Receive( const U8* _senderMACAddress, const U8* _payload, U32 _payloadSize ) {

	// Check we received the expected header
	if ( _payloadSize < ESP_NOW_MAX_DATA_LEN ) {
		m_lastReceivedPacketStatus = INVALID_PAYLOAD_SIZE;
Serial.printf( "Received unexpected payload size %d\n", _payloadSize );
		return;	// Not long enough
	}
	if ( memcmp( _payload, &m_header, sizeof(U16) ) ) {
		m_lastReceivedPacketStatus = INVALID_HEADER;
Serial.printf( "Unexpected header 0x%04X\n", *((U16*) _payload) );
		return;	// Not the expected header...
	}

	_payload += sizeof(U16);	// Skip header

	// Read receiver mask ID and packet ID
	U32	packetID = *((U32*) _payload);
	_payload += sizeof(U32);

	// Check if this payload is addressed to us
	U8	receiverMaskID = U8( packetID );
	if ( (receiverMaskID & m_receiverMaskID) == 0	// Just a regular mask?
	 && receiverMaskID != m_receiverMaskID ) {		// Or is it addressed to the Central (ID 0) and we are it?
		m_lastReceivedPacketStatus = NOT_FOR_US;
Serial.println( "Not for us!" );
		return;
	}

	// Read packet ID & check for lost packets
	packetID >>= 8;
	packetID |= m_lastReceivedPacketID & 0xFF000000UL;	// Always re-use last packet's MSB

	// Fix ID looping after 2^24 packets:
	//  • Our last received packet ID is on 32 bits and could be 0xHH^^^^^^ where HH € [0,255] and 0x00^^^^^^ is a very high 24-bits number (close to looping)
	//	• Our new packet ID is suddendly jumping back down to a very low number like 0xHHvvvvvv < 0xHH^^^^^^
	if ( packetID < m_lastReceivedPacketID ) {
		packetID += 0x01000000UL;	// To fix this, we simply add one to the MSB of the packetID...
	}

	U32	lostPacketsCount = packetID - m_lastReceivedPacketID - 1;
	if ( lostPacketsCount != 0 && lostPacketsCount < 10 ) {
//if ( lostPacketsCount > 1 && lostPacketsCount < 10 ) {
//	Serial.printf( "Lost %d packets!\n", lostPacketsCount );	// We very rarely lose more than a single packet (if connection is okay)
//}
		U32		lostSamplesCount = lostPacketsCount * SAMPLES_PER_PACKET;

		#if 1	// Interpolate from last sample value
			Sample&	lastSample = m_buffer[(m_sampleIndexWrite + m_bufferSize-1) % m_bufferSize];	// Last sample we received
			Sample&	newSample = (Sample&) *_payload;												// Next valid sample in our new payload
//Sample	newSample = { .left = 0, .right = 0 };

#ifdef SIMULATE_WAVE_FORM_RECEIVER	// Simulate interpolation of the clear wave form with lost packets
s_sampleIndex += lostSamplesCount;
S16	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex ) );
newSample = (Sample) { .left = temp, .right = temp };
#endif

			S32	left0 = lastSample.left;
			S32	dLeft = (S32) newSample.left - left0;
			S32	right0 = lastSample.right;
			S32	dRight = (S32) newSample.right - right0;

			for ( U32 lostSampleIndex=0; lostSampleIndex < lostSamplesCount; lostSampleIndex++ ) {
				U32	bufferSampleIndex = (m_sampleIndexWrite + lostSampleIndex) % m_bufferSize;
				m_buffer[bufferSampleIndex].left = S16( left0 + (lostSampleIndex * dLeft) / lostSamplesCount );
				m_buffer[bufferSampleIndex].right = S16( right0 + (lostSampleIndex * dRight) / lostSamplesCount );


#ifdef SIMULATE_WAVE_FORM_RECEIVER	// Simulate the complete wave form (as if packets hadn't been lost)
//S32	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );
//S32	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / 22050.0f) * s_sampleIndex++ ) );
//m_buffer[bufferSampleIndex].left = temp;
//m_buffer[bufferSampleIndex].right = temp;
#endif


			}
			m_sampleIndexWrite += lostSamplesCount;

		#elif 0	// Write 0 in place of lost packets => BAD!
			U32	lostSamplesCount = min( m_bufferSize, lostPacketsCount * SAMPLES_PER_PACKET );
			U32	sampleBufferIndex = m_sampleIndexWrite % m_bufferSize;
			U32	samplesCountToEnd = m_bufferSize - sampleBufferIndex;
			if ( lostSamplesCount <= samplesCountToEnd ) {
				memset( m_buffer + sampleBufferIndex, 0, lostSamplesCount * sizeof(Sample) );
			} else {
				memset( m_buffer + sampleBufferIndex, 0, samplesCountToEnd * sizeof(Sample) );
				memset( m_buffer, 0, (lostSamplesCount - samplesCountToEnd) * sizeof(Sample) );
			}
			m_sampleIndexWrite += lostPacketsCount * SAMPLES_PER_PACKET;
//			for ( U32 sampleIndex=SAMPLES_PER_PACKET*lostPacketsCount; sampleIndex > 0; sampleIndex--, m_sampleIndexWrite++ ) {
//				memset( m_buffer + (m_sampleIndexWrite % m_bufferSize), 0, sizeof(Sample) );
//			}
		#elif 0	// Duplicate lost packets
TODO!
		#elif 0	// Skip packets => BAD!
			// Sound is worse if we skip packets! :/

#ifdef SIMULATE_WAVE_FORM_RECEIVER	// Simulate the complete wave form (as if packets hadn't been lost)
for ( U32 i=0; i < lostSamplesCount; i++ ) {
//	S32	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );
S32	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / 22050.0f) * s_sampleIndex++ ) );
	m_buffer[(m_sampleIndexWrite + i) % m_bufferSize].left = temp;
	m_buffer[(m_sampleIndexWrite + i) % m_bufferSize].right = temp;
}
#endif

			m_sampleIndexWrite += lostSamplesCount;
		#endif

		m_lostPacketsCount += lostPacketsCount;
		m_receivedPacketsCount += lostPacketsCount;
//Serial.printf( "Lost packet %02X / %02X = %d\n", m_lastReceivedPacketID, packetID, lostPacketsCount );
	}
//Serial.printf( "Packet ID %02X / %02X\n", m_lastReceivedPacketID, packetID );

	// Read samples
	U32	bufferIndex = m_sampleIndexWrite % m_bufferSize;
	U32	samplesCountToEnd = m_bufferSize - bufferIndex;
	if ( SAMPLES_PER_PACKET <= samplesCountToEnd ) {	// Transfer in a single shot
		memcpy( m_buffer + bufferIndex, _payload, SAMPLES_PER_PACKET * sizeof(Sample) );
	} else {	// Split in 2
		memcpy( m_buffer + bufferIndex, _payload, samplesCountToEnd * sizeof(Sample) );
		memcpy( m_buffer, _payload + samplesCountToEnd * sizeof(Sample), (SAMPLES_PER_PACKET - samplesCountToEnd) * sizeof(Sample) );
	}


#ifdef SIMULATE_WAVE_FORM_RECEIVER
for ( U32 i=0; i < SAMPLES_PER_PACKET; i++ ) {
//	S32	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );
S32	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / 22050.0f) * s_sampleIndex++ ) );
	m_buffer[(m_sampleIndexWrite + i) % m_bufferSize].left = temp;
	m_buffer[(m_sampleIndexWrite + i) % m_bufferSize].right = temp;
}
#endif


	m_sampleIndexWrite += SAMPLES_PER_PACKET;

	m_receivedPacketsCount++;
	U32	oldLastReceivedPacketID = m_lastReceivedPacketID;
	m_lastReceivedPacketID = packetID;

	// Notify
	if ( m_onPacketsReceivedCallback != NULL ) {
		(*m_onPacketsReceivedCallback)( oldLastReceivedPacketID );
	}
}
#endif

void	ReceiveCallback( const U8* _senderMACAddress, const U8* _data, int _dataLength ) {
	gs_instance->Receive( _senderMACAddress, _data, _dataLength );
}


///////////////////////////////////////////////////////////////////////////
// Transmitter
///////////////////////////////////////////////////////////////////////////
//
void	TransportESPNow_Transmitter::Init( U8 _WiFiChannel, U32 _samplingRate ) {
	TransportESPNow_Base::Init( _WiFiChannel, _samplingRate );
}

void	TransportESPNow_Transmitter::SendPacket( ISampleSource& _sampleSource, U32 _packetID, U8 _receiverMaskID ) {
	// Write header
	*((U16*) m_buffer) = m_header;

	// Write packet info
	_packetID <<= 8;				// Ignore MSB
	_packetID |= _receiverMaskID;	// Replace by receiver mask ID
	*((U32*) (m_buffer+2)) = _packetID;

	// We have 250 - 4 = 246 bytes available for payload, each sample is 4 bytes, so we can only ask for SAMPLES_PER_PACKET samples at a time
	U32	requestedSamplesCount = _sampleSource.GetChannelsCount() == ISampleSource::CHANNELS::STEREO ? SAMPLES_PER_PACKET : 2*SAMPLES_PER_PACKET;
	_sampleSource.GetSamples( m_samplingRate, (Sample*) (m_buffer + 6), requestedSamplesCount );


#ifdef SIMULATE_WAVE_FORM_TRANSMITTER
// Simulate a 1KHz sine wave
Sample*		sample = (Sample*) (m_buffer + 6);
for ( U32 i=0; i < SAMPLES_PER_PACKET; i++, sample++ ) {
//	S32	temp = S16( SIMULATE_WAVE_FORM_TRANSMITTER * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndex++ ) );
S32	temp = S16( SIMULATE_WAVE_FORM_TRANSMITTER * sin( 2*3.14159265358979f * (1000.0f / 22050.0f) * s_sampleIndex++ ) );
	sample->left = temp;
	sample->right = temp;
}
#endif


	// Send the packet
	esp_err_t	result = esp_now_send( broadcastAddress, m_buffer, ESP_NOW_MAX_DATA_LEN );
	if ( result != ESP_OK ) {
		Serial.printf( "Failed to send: %s\n", esp_err_to_name(result) );
//m_sampleIndex = 0;
//return;
	}

//Serial.printf( "Sent %d bytes", m_headerSize + m_sampleIndex );
	
	m_sentPacketsCount++;

// Monitor packets by blinking the LED every 128 packets...
digitalWrite( PIN_LED_RED, (m_sentPacketsCount & 0x80) != 0 );
}
