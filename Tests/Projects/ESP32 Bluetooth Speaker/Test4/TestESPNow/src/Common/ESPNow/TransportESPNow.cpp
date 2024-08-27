#include "../Global.h"
#include "TransportESPNow.h"

#include <esp_now.h>
#include <esp_wifi.h>

const U8	broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

//#define SIMULATE_WAVE_FORM_RECEIVER		1024	// Should sound like a clear 1KHz sound if all packets were received correctly...
//#define SIMULATE_WAVE_FORM_TRANSMITTER	1024	// Should sound like a clear 1KHz sound if all packets were transmitted correctly...

#if defined(SIMULATE_WAVE_FORM_RECEIVER)
static U32	s_sampleIndexReceive = 0;
#endif
#if defined(SIMULATE_WAVE_FORM_TRANSMITTER)
static U32	s_sampleIndexTransmit = 0;
#endif


// @TODO: Restore STA mode if failing in AccessPoint mode
#if 0// defined(BUILD_CENTRAL)
wifi_mode_t			mode = WIFI_MODE_AP;
wifi_interface_t	interface = WIFI_IF_AP;
#else
wifi_mode_t			mode = WIFI_MODE_STA;
wifi_interface_t	interface = WIFI_IF_STA;
#endif

//wifi_phy_rate_t		WiFiRate = wifi_phy_rate_t::WIFI_PHY_RATE_2M_L );	// 2 Mbps with long preamble <= Lots of ESPNOW_NO_MEM errors!
//wifi_phy_rate_t		WiFiRate = wifi_phy_rate_t::WIFI_PHY_RATE_5M_L );	// 5.5 Mbps with long preamble <= Less packets lost! (~60 packets over 723)
wifi_phy_rate_t		WiFiRate = wifi_phy_rate_t::WIFI_PHY_RATE_11M_L;	// 11 Mbps with long preamble <= Can't really see the difference? :/

//wifi_phy_rate_t		WiFiRate = wifi_phy_rate_t::WIFI_PHY_RATE_5M_S );	// 5.5 Mbps with short preamble <= Lot less of ESPNOW_NO_MEM errors but they sometimes happens in bursts...
//wifi_phy_rate_t		WiFiRate = wifi_phy_rate_t::WIFI_PHY_RATE_11M_S;	// 11 Mbps with short preamble <= Can't really see the difference? :/
//wifi_phy_rate_t		WiFiRate = wifi_phy_rate_t::WIFI_PHY_RATE_54M;			// 54Mbps <= Lots of packets lost!!

void	TransportESPNow_Base::ConfigureWiFi( U8 _WiFiChannel ) {


//WiFi.enableLongRange( true );


	WiFi.mode( mode );
	WiFi.disconnect();	// We're using ESP-Now
	Serial.println( str( "MAC Address is: %s", WiFi.macAddress().c_str() ) );

	// Ensure we're at max power
//wifi_power_t power = WiFi.getTxPower();
//Serial.printf( "WiFi Power: %d\n", power );
	WiFi.setTxPower( WIFI_POWER_19_5dBm );

	#if defined(BUILD_CENTRAL)
		// Ensure the channel is free (otherwise a lot of interference and ESP_NOW queue errors can occur!)
		const char*	SSID = TransportESPNow_Base::CheckWiFiChannelUnused( _WiFiChannel );
		ERROR( SSID, str( "The WiFi network \"%s\" is operating on our WiFi channel %d!", SSID ? SSID : "", _WiFiChannel ) );
	#endif


// Debug WiFi channels
//U8	channels[11];
//TransportESPNow_Base::DumpWiFiScan();
//TransportESPNow_Base::ScanWifiChannels( channels, true );
//ERROR( channels[ESP_NOW_WIFI_CHANNEL-1], "A WiFi network is operating on our WiFi channel!" );

//	esp_wifi_set_protocol( interface, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR );
//	esp_wifi_set_protocol( interface, WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR );
	esp_wifi_set_protocol( interface, WIFI_PROTOCOL_LR );

//	esp_wifi_set_bandwidth( interface, WIFI_BW_HT20 );
	esp_wifi_set_bandwidth( interface, WIFI_BW_HT40 );	// Not advised in the 2.4GHz band? (source: https://support.huawei.com/enterprise/en/knowledge/EKB1000079063)

//typedef enum
//{
//WIFI_PHY_MODE_LR,   /**< PHY mode for Low Rate */
//WIFI_PHY_MODE_11B,  /**< PHY mode for 11b */
//WIFI_PHY_MODE_11G,  /**< PHY mode for 11g */
//WIFI_PHY_MODE_HT20, /**< PHY mode for Bandwidth HT20 */
//WIFI_PHY_MODE_HT40, /**< PHY mode for Bandwidth HT40 */
//WIFI_PHY_MODE_HE20, /**< PHY mode for Bandwidth HE20 */
//} wifi_phy_mode_t;

	// Set Wifi channel
	esp_wifi_set_promiscuous( true );
	esp_wifi_set_channel( _WiFiChannel, WIFI_SECOND_CHAN_NONE );
	esp_wifi_set_promiscuous( false );

	// NOTE on promiscuous mode:
	//	In computer networking, promiscuous mode is a mode for a wired network interface controller (NIC) or wireless network interface controller (WNIC)
	//	 that causes the controller to pass all traffic it receives to the central processing unit (CPU) rather than passing only the frames that the controller
	//	 is specifically programmed to receive. This mode is normally used for packet sniffing that takes place on a router or on a computer connected to a
	//	 wired network or one being part of a wireless LAN.[1] Interfaces are placed into promiscuous mode by software bridges often used with hardware virtualization.	


//	// Source: https://forum.arduino.cc/t/esp32-and-esp-now-for-audio-streaming-slow-acknowledge-from-receiver/1055192/6
//	// Set device as a Wi-Fi Station
//	esp_netif_init();
//	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//	esp_wifi_init(&cfg);
//	esp_wifi_set_mode(WIFI_MODE_STA);
//	esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_ps(WIFI_PS_NONE);
//	esp_wifi_start();
//
//	// Init ESP-NOW
//	if ( esp_now_init() != ESP_OK ) {
//		Serial.println( "Error initializing ESP-NOW" );
//		return;
//	}
//	esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_54M);
}


//@TODO: Check WiFi home channel and avoid?
// Cf. => https://www.electrosoftcloud.com/en/esp32-wifi-and-esp-now-simultaneously/
//
void	TransportESPNow_Base::Init( U32 _samplingRate ) {
	m_samplingRate = _samplingRate;


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

	result = esp_wifi_config_espnow_rate( peerInfo.ifidx, WiFiRate );
	ERROR( result != ESP_OK, str( "Failed to setup WiFi broadcast rate: %s", esp_err_to_name(result) ) );
	Serial.println( "Successfully set WiFi rate to 5.5 Mbps" );

	Serial.printf( "TransportESPNow Initialized for %d Hz!\n", m_samplingRate );
}

// Source code from https://deepbluembedded.com/esp32-wifi-scanner-example-arduino/
U32	WiFiScan() {
	return WiFi.scanNetworks( false, true, true, 90 );
}

const char*	TransportESPNow_Base::CheckWiFiChannelUnused( U8 _channel ) {
	static char	SSID[33];

	U32	networksCount = WiFiScan();

	for ( U32 networkIndex=0; networkIndex < networksCount; networkIndex++ ) {
	    wifi_ap_record_t*	record = (wifi_ap_record_t*) WiFiScanClass::getScanInfoByIndex( networkIndex );
		if ( record->primary == _channel ) {
			// Found a network on this channel!
			memcpy( SSID, record->ssid, 33 );
			WiFi.scanDelete();
			return SSID;
		}
	}

	WiFi.scanDelete();

	return NULL;
}

U32	TransportESPNow_Base::DumpWiFiScan( bool _deleteScanOnExit ) {
	U32	networksCount = WiFiScan();
	Serial.print( "Scan done => " );
	if ( networksCount == 0 ) {
		Serial.println( "no networks found" );
	} else {
		Serial.print( networksCount );
		Serial.println( " networks found" );
		Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
		for (int i = 0; i < networksCount; ++i) {
			// Print SSID and RSSI for each network found
			Serial.printf("%2d",i + 1);
			Serial.print(" | ");
			Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
			Serial.print(" | ");
			Serial.printf("%4d", WiFi.RSSI(i));
			Serial.print(" | ");
			Serial.printf("%2d", WiFi.channel(i));
			Serial.print(" | ");
			switch (WiFi.encryptionType(i))
			{
			case WIFI_AUTH_OPEN:
				Serial.print("open");
				break;
			case WIFI_AUTH_WEP:
				Serial.print("WEP");
				break;
			case WIFI_AUTH_WPA_PSK:
				Serial.print("WPA");
				break;
			case WIFI_AUTH_WPA2_PSK:
				Serial.print("WPA2");
				break;
			case WIFI_AUTH_WPA_WPA2_PSK:
				Serial.print("WPA+WPA2");
				break;
			case WIFI_AUTH_WPA2_ENTERPRISE:
				Serial.print("WPA2-EAP");
				break;
			case WIFI_AUTH_WPA3_PSK:
				Serial.print("WPA3");
				break;
			case WIFI_AUTH_WPA2_WPA3_PSK:
				Serial.print("WPA2+WPA3");
				break;
			case WIFI_AUTH_WAPI_PSK:
				Serial.print("WAPI");
				break;
			default:
				Serial.print("unknown");
			}
			Serial.println();
			delay(10);
		}
	}
	Serial.println("");

	if ( _deleteScanOnExit ) {
		WiFi.scanDelete();	// Delete the scan result to free memory for code below.
	}

	return networksCount;
}

U32	TransportESPNow_Base::ScanWifiChannels( U8 _channels[11], bool _dump ) {
	memset( _channels, 0, 11 );
	U32	networksCount = 0;
	if ( _dump ) {
		networksCount = DumpWiFiScan( false );
	} else {
		networksCount = WiFiScan();
	}

	for ( U32 networkIndex=0; networkIndex < networksCount; networkIndex++ ) {
		U8	channelID = WiFi.channel( networkIndex );
		ERROR( channelID < 1 || channelID > 11, "WiFi channel ID out of range!" );
		_channels[channelID-1] = 1;	// This channel is used!
	}
	WiFi.scanDelete();

//esp_wifi_set_max_tx_power => boost?

	return networksCount;
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

bool	TransportESPNow_Receiver::Init( U8 _receiverMaskID, U32 _samplingRate, CHANNELS _channelsCount, float _preLoadDelay ) {
	TransportESPNow_Base::Init( _samplingRate );

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
	if ( !m_time->HasStarted() || m_blockPackets )
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
	if (  (receiverMaskID & m_receiverMaskID) == 0	// Just a regular mask?
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

	if ( packetID == m_lastReceivedPacketID ) {
		return;	// Ignore packet as we've already received it! (this happens when enabling packet doubling)
	}

	U32	lostPacketsCount = packetID - m_lastReceivedPacketID - 1;
	if ( lostPacketsCount != 0 ) {
		#if 0 // Creates many scratches... Don't know why.
		if ( lostPacketsCount < 10 ) {
			// Interpolate lost packets
			if ( GetChannelsCount() == STEREO ) {
				U32		lostSamplesCount = lostPacketsCount * SAMPLES_PER_PACKET;

				Sample&	lastSample = m_buffer[(m_sampleIndexWrite + m_bufferSize-1) % m_bufferSize];	// Last sample we received
				Sample&	newSample = *((Sample*) _payload);												// Next valid sample in our new payload

				S32	left0 = lastSample.left;
				S32	dLeft = (S32) newSample.left - left0;
				S32	right0 = lastSample.right;
				S32	dRight = (S32) newSample.right - right0;

				for ( U32 lostSampleIndex=0; lostSampleIndex < lostSamplesCount; lostSampleIndex++ ) {
					U32	bufferSampleIndex = (m_sampleIndexWrite + lostSampleIndex) % m_bufferSize;
					m_buffer[bufferSampleIndex].left = S16( left0 + (lostSampleIndex * dLeft) / lostSamplesCount );
					m_buffer[bufferSampleIndex].right = S16( right0 + (lostSampleIndex * dRight) / lostSamplesCount );
				}
				m_sampleIndexWrite += lostSamplesCount;
			} else {
				U32		lostSamplesCount = lostPacketsCount * 2*SAMPLES_PER_PACKET;
				S16*	buffer = (S16*) m_buffer;

				S16	lastSample = buffer[(m_sampleIndexWrite + m_bufferSize-1) % m_bufferSize];	// Last sample we received
				S16	newSample = *((S16*) _payload);												// Next valid sample in our new payload
				S32	delta = (S32) newSample - lastSample;

				for ( U32 lostSampleIndex=0; lostSampleIndex < lostSamplesCount; lostSampleIndex++ ) {
					U32	bufferSampleIndex = (m_sampleIndexWrite + lostSampleIndex) % m_bufferSize;
					buffer[bufferSampleIndex] = S16( lastSample + (lostSampleIndex * delta) / lostSamplesCount );
				}
				m_sampleIndexWrite += lostSamplesCount;
			}
		}
		#endif

		m_lostPacketsCount += lostPacketsCount;
		m_receivedPacketsCount += lostPacketsCount;
//Serial.printf( "Lost packet %02X / %02X = %d\n", m_lastReceivedPacketID, packetID, lostPacketsCount );
	}
//Serial.printf( "Packet ID %02X / %02X\n", m_lastReceivedPacketID, packetID );


#ifdef SIMULATE_WAVE_FORM_RECEIVER
if ( m_channelsCount == ISampleSource::STEREO ) {
	Sample*	sample = (Sample*) _payload;
	for ( U32 i=0; i < SAMPLES_PER_PACKET; i++, sample++ ) {
		S16	temp = FastSine( s_sampleIndexReceive++ * (16384 * 1000 / m_samplingRate) ) / 8;
		sample->left = temp;
		sample->right = temp;
	}
} else {
	S16*	sample = (S16*) _payload;
	for ( U32 i=0; i < 2*SAMPLES_PER_PACKET; i++ ) {
		*sample++ = FastSine( s_sampleIndexReceive++ * (16384 * 1000 / m_samplingRate) ) / 8;
	}
}
// Too slow => on tombe à 40KHz quand on enable cette ligne?
//Sample*	sample = (Sample*) _payload;
//for ( U32 i=0; i < SAMPLES_PER_PACKET; i++, sample++ ) {
//	S32	temp = S16( SIMULATE_WAVE_FORM_RECEIVER * sin( 2*3.14159265358979f * (1000.0f / m_samplingRate) * s_sampleIndexReceive++ ) );
//	sample->left = temp;
//	sample->right = temp;
//}
#endif


	// Write samples
	U32	receivedSamplesCount = m_channelsCount == ISampleSource::STEREO ? SAMPLES_PER_PACKET : 2*SAMPLES_PER_PACKET;
	WriteSamples( (Sample*) _payload, receivedSamplesCount );

	m_receivedPacketsCount++;
	U32	oldLastReceivedPacketID = m_lastReceivedPacketID;
	m_lastReceivedPacketID = packetID;

	// Notify
	if ( m_onPacketsReceivedCallback != NULL ) {
		(*m_onPacketsReceivedCallback)( oldLastReceivedPacketID );
	}
}

void	ReceiveCallback( const U8* _senderMACAddress, const U8* _data, int _dataLength ) {
	gs_instance->Receive( _senderMACAddress, _data, _dataLength );
}


///////////////////////////////////////////////////////////////////////////
// Transmitter
///////////////////////////////////////////////////////////////////////////
//
void	SendPacketsTask( void* _param );

void	TransportESPNow_Transmitter::Init( U32 _samplingRate ) {
	TransportESPNow_Base::Init( _samplingRate );
}

void	TransportESPNow_Transmitter::StartAutoSendTask( U8 _taskPriority, ISampleSource& _sampleSource, U8 _receiverMaskID ) {
	m_sampleSource = &_sampleSource;
	m_receiverMaskID = _receiverMaskID;

	TaskHandle_t sendPacketsTaskHandle;
	xTaskCreate( SendPacketsTask, "SendPacketsTask", 2048, this, _taskPriority, &sendPacketsTaskHandle );
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
if ( _sampleSource.GetChannelsCount() == ISampleSource::STEREO ) {
	Sample*	sample = (Sample*) (m_buffer + 6);
	for ( U32 i=0; i < requestedSamplesCount; i++, sample++ ) {
	//	S16	temp = s_sampleIndexTransmit++ & 0x10 ? 2048 : -2048;
		S16	temp = FastSine( s_sampleIndexTransmit++ * (16384 * 1000 / m_samplingRate) ) / 8;
		sample->left = temp;
		sample->right = temp;
	}
} else {
	S16*	sample = (S16*) (m_buffer + 6);
	for ( U32 i=0; i < requestedSamplesCount; i++ ) {
		*sample++ = FastSine( s_sampleIndexTransmit++ * (16384 * 1000 / m_samplingRate) ) / 8;
	}
}
#elif 0
// Simulate a 1KHz sine wave
Sample*		sample = (Sample*) (m_buffer + 6);
for ( U32 i=0; i < SAMPLES_PER_PACKET; i++, sample++ ) {
//	S32	temp = S16( SIMULATE_WAVE_FORM_TRANSMITTER * sin( 2*3.14159265358979f * (1000.0f / 44100.0f) * s_sampleIndexTransmit++ ) );
S32	temp = S16( SIMULATE_WAVE_FORM_TRANSMITTER * sin( 2*3.14159265358979f * (1000.0f / 22050.0f) * s_sampleIndexTransmit++ ) );
	sample->left = temp;
	sample->right = temp;
}
#endif

	// Send the packet
	SendRawPacket( m_buffer );
}

void	TransportESPNow_Transmitter::SendRawPacket( const U8 _packet[ESP_NOW_MAX_DATA_LEN] ) {
	esp_err_t	result = esp_now_send( broadcastAddress, _packet, ESP_NOW_MAX_DATA_LEN );
	if ( result != ESP_OK ) {
		Serial.printf( "Failed to send: %s\n", esp_err_to_name(result) );
//m_sampleIndex = 0;
//		return;
	}

	#ifdef ENABLE_PACKET_DOUBLING
		result = esp_now_send( broadcastAddress, _packet, ESP_NOW_MAX_DATA_LEN );
		if ( result != ESP_OK ) {
			Serial.printf( "Failed to send SECOND PACKET: %s\n", esp_err_to_name(result) );
//			return;
		}
	#endif

//Serial.printf( "Sent %d bytes", m_headerSize + m_sampleIndex );
	
	m_sentPacketsCount++;

// Monitor packets by blinking the LED every 128 packets...
digitalWrite( PIN_LED_RED, (m_sentPacketsCount & 0x80) != 0 );
}

void	SendPacketsTask( void* _param ) {
	TransportESPNow_Transmitter*	that = (TransportESPNow_Transmitter*) _param;

	const ITimeReference&	time = *that->m_time;
	ISampleSource&			sampleSource = *that->m_sampleSource;

	U8	receiverMaskID = that->m_receiverMaskID;
	U64	sendDeltaTime = (1000000ULL * (sampleSource.GetChannelsCount() == ISampleSource::STEREO ? TransportESPNow_Base::SAMPLES_PER_PACKET : 2*TransportESPNow_Base::SAMPLES_PER_PACKET))
					  / U64( that->m_samplingRate );	// How much time represents that many samples given our sampling rate?

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

		if ( !that->m_blockPackets ) {
			U32	packetID = that->m_sentPacketsCount;	// Use the transport's packet counter as packet ID
			that->SendPacket( sampleSource, packetID, receiverMaskID );
		}

		timeNextSend += sendDeltaTime;
	}
}
