////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sound Transmission via Bluetooth to the EMEET M0 Plus
// We make the ESP32 an audio source and attempt to connect to the EMEET to output some audio...
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#if 1

#include "Global.h"

#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include "BluetoothA2DPSource.h"

#include <BluetoothSerial.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "I2SOutput/I2SOutput.h"
#include "I2SOutput/WAVFileSampler.h"

//#include "SPIFFS.h"
//#include <FS.h>


// Make sure Bluetooth is enabled & available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif


#define TEST_SOURCE 1  // Define this to play sound to an A2DP sink device (use 1 to play the WAV file, 0 to play a sine wave)
//#define TEST_SINK
//#define TEST_I2S_OUTPUT // Define this to play sound to an MAX98357A I2S amplifier

#if TEST_SOURCE || defined(TEST_I2S_OUTPUT)
  #define USE_WAV 1
#else
  #define USE_WAV 0
#endif

char  str::ms_globalBuffer[256];
char* str::ms_globalPointer = str::ms_globalBuffer;


//////////////////////////////////////////////////////////////
#ifdef TEST_I2S_OUTPUT  // Uses the code from Atomic14

#define PIN_I2S_DOUT  22  // Data out
#define PIN_I2S_BCLK  26  // Bits clock
#define PIN_I2S_LRC   25  // Left/Right Select

I2SOutput       output;
WAVFileSampler  sampleSourceI2S;

void  InitI2S() {
  if ( !sampleSourceI2S.Init( "/Alarm03.wav" ) ) {
    Serial.println( "An error occurred while reading the WAV file for I2S source..." );
    return;
  }

  sampleSourceI2S.SetVolume( 0.125f );

  i2s_pin_config_t  i2sPins = {
      .bck_io_num = PIN_I2S_BCLK,
      .ws_io_num = PIN_I2S_LRC,
      .data_out_num = PIN_I2S_DOUT,
      .data_in_num = -1
  };

  output.start( I2S_NUM_0, i2sPins, &sampleSourceI2S );

  Serial.println( "I2S Initialized with WAV file" );
}

#endif

#ifdef TEST_SOURCE

BluetoothA2DPSource a2dp_source;
#if USE_WAV
  WAVFileSampler      sampleSourceA2DP;
#endif

// The supported audio codec in ESP32 A2DP is SBC. SBC audio stream is encoded from PCM data normally formatted as 44.1kHz sampling rate, two-channel 16-bit sample data
//U32   WAVSampleIndex = 0;

int32_t get_data_frames( Frame* frame, int32_t frame_count ) {
  #if USE_WAV
    sampleSourceA2DP.getFrames( (Frame_t*) frame, frame_count );
/*    for ( int sample=0; sample < frame_count; sample++, frame++ ) {
//      WAVFile.read( (U8*) frame, sizeof(Frame) );
      WAVFile.read( (U8*) &frame->channel1, sizeof(uint16_t) );
      WAVFile.read( (U8*) &frame->channel2, sizeof(uint16_t) );
      WAVSampleIndex++;

      if ( WAVSampleIndex == WAVSamplesCount ) {
        // Restart...
Serial.println( str( "Loop! Offset %d (Min = 0x%02X - 0x%02x | Max = 0x%02X - 0x%02x)", WAVFile.position(), minFrame.channel1, minFrame.channel2, maxFrame.channel1, maxFrame.channel2 ) );
        WAVFile.seek( WAVStartOffset );
        WAVSampleIndex = 0;
      }

      // Keep min/max values
      if ( frame->channel1 > maxFrame.channel1 ) maxFrame.channel1 = frame->channel1;
      if ( frame->channel2 > maxFrame.channel2 ) maxFrame.channel2 = frame->channel2;
      if ( frame->channel1 < minFrame.channel1 ) minFrame.channel1 = frame->channel1;
      if ( frame->channel2 < minFrame.channel2 ) minFrame.channel2 = frame->channel2;
    }
*/

/*    static U32  sampleIndex = 0;
    for (int sample = 0; sample < frame_count; ++sample, sampleIndex++ ) {
//      frame[sample].channel1 = ((sampleIndex >> 8) & 1) ? 32767 : -32768;
      frame[sample].channel1 = S16( 10000.0 * sin( sampleIndex * 6.28 / 20.0 ) );
      frame[sample].channel2 = frame[sample].channel1;
    }
*/

/*    S16 min = 32767, max = -32768;
    for ( int sample = 0; sample < frame_count; sample++ ) {
      S16 left = frame[sample].channel1;
      min = left < min ? left : min;
      max = left > max ? left : max;
    }
Serial.print( str( "min=%d max=%d | ", min, max ) );
*/
  #else
    static float m_time = 0.0;

    const float c3_frequency = 1300.81f;

    float m_amplitude = 10000.0;  // -32,768 to 32,767
    float m_deltaTime = 1.0 / 44100.0;
    float m_phase = 0.0;
    float pi_2 = PI * 2.0;
    // fill the channel data
    for (int sample = 0; sample < frame_count; ++sample) {
        float angle = pi_2 * c3_frequency * m_time + m_phase;
        frame[sample].channel1 = m_amplitude * sin(angle);
        frame[sample].channel2 = frame[sample].channel1;
        m_time += m_deltaTime;
    }
    #endif

    // to prevent watchdog
//    delay(1);
    vTaskDelay( 1 );

    return frame_count;
}

// Return true to connect, false will continue scanning
bool isValid( const char* ssid, esp_bd_addr_t address, int rssi ){
   Serial.println( str( "  A2DP Device found -> SSID: %s (RSSI = %d)", ssid, rssi ) );
   return true;
}

// for esp_a2d_connection_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv426esp_a2d_connection_state_t
void connection_state_changed( esp_a2d_connection_state_t state, void* ptr ) {
  Serial.println( a2dp_source.to_str(state) );
}

TwoChannelSoundData soundData( true );
U8* rawSoundData = NULL;

void  InitA2DPSource() {
  Serial.println( "InitA2DPSource()" );
  
  #if USE_WAV
//    if ( !sampleSourceA2DP.Init( "/Alarm03.wav", true ) ) {	// File is too large to preload!
    if ( !sampleSourceA2DP.Init( "/Alarm03.wav", true ) ) {
      Serial.println( "An error occurred while reading the WAV file for A2DP source..." );
      return;
    }
  #endif

  /////////////////////////////////////////////////////////////////////////
  // Start an A2DP source
  a2dp_source.set_auto_reconnect( true );
  a2dp_source.set_ssid_callback( isValid );
  a2dp_source.set_on_connection_state_changed( connection_state_changed );

  a2dp_source.set_volume( 30 );  // Example sets 30 by default

//  a2dp_source.start("LEXON MINO L", get_data_frames);  
//  a2dp_source.start( "EMEET OfficeCore M0 Plus", get_data_frames );
//  a2dp_source.start( "Mon Cul A2DP Source", get_data_frames );

  // Custom callback
  a2dp_source.start( "SANWU Audio", get_data_frames );

/*
  // Read data once, like in the piano example... But that seems to crash...
Serial.println( str( "Allocating %d bytes...", sampleSourceA2DP.m_samplesCount * 4 ) );
//  rawSoundData = new U8[sampleSourceA2DP.m_samplesCount * 4];
  rawSoundData = (U8*) malloc( sampleSourceA2DP.m_samplesCount * 4 );
Serial.println( str( "Reading %d bytes from file...", sampleSourceA2DP.m_samplesCount * 4 ) );
  sampleSourceA2DP.m_file.read( rawSoundData, sampleSourceA2DP.m_samplesCount * 4 );
Serial.println( str( "Setting data for sound data source" ) );
  soundData.setData( (Frame*) rawSoundData, sampleSourceA2DP.m_samplesCount );
//  delete[] rawSoundData;

Serial.println( str( "Writing data..." ) );
  a2dp_source.write_data( &soundData );
  a2dp_source.start( "ESP32 Source" );
//*/

  Serial.println( "A2DP Source started..." );
}

#elif defined(TEST_SINK)

I2SStream i2s;
BluetoothA2DPSink   a2dp_sink( i2s );

void  OnDataReceived() {
  Serial.println( "  Data received!" );
}

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
  if (id == ESP_AVRC_MD_ATTR_PLAYING_TIME) {
    uint32_t playtime = String((char*)text).toInt();
    Serial.printf("==> Playing time is %d ms (%d seconds)\n", playtime, (int)round(playtime/1000.0));
  }
}

bool  ValidateAddress( esp_bd_addr_t remote_bda ) {
  Serial.print("  Received address validation callback for device address: " );
  for ( int i=0; i < ESP_BD_ADDR_LEN; i++ ) {
    Serial.print( str( "%02X", remote_bda[i] ) );
  }
  Serial.println();
  
  return true;
}

void StreamReader( const uint8_t* _data, uint32_t _dataLength ) {
  Serial.println( str( "  %d bytes received!", _dataLength ) );
}

#else

BluetoothSerial BT;

void  BTAdvertisedDeviceFound( BTAdvertisedDevice* device ) {
  Serial.println( str( "  Device found -> %s (RSSI = %d)", device->getName().c_str(), device->getRSSI() ) );
}

#endif

void setup() {
  Serial.begin( 115200 );
  Serial.println( "Serial ready" );

  if ( !SPIFFS.begin( true ) ) {
    Serial.println( "An Error has occurred while mounting SPIFFS!" );
    return;
  }


//*
Serial.println( "Opening file..." );
File  file = SPIFFS.open( "/Alarm03.wav" );
if ( file )
  Serial.println( "SUCCESS!" );
else
  Serial.println( "FAILED!" );
file.close();
//*/

#ifdef TEST_I2S_OUTPUT
  InitI2S();
#endif

#ifdef TEST_SOURCE
  InitA2DPSource();

#elif defined(TEST_SINK)

  a2dp_sink.set_default_bt_mode(ESP_BT_MODE_BTDM);
  a2dp_sink.set_avrc_metadata_attribute_mask(ESP_AVRC_MD_ATTR_TITLE | ESP_AVRC_MD_ATTR_ARTIST | ESP_AVRC_MD_ATTR_ALBUM | ESP_AVRC_MD_ATTR_PLAYING_TIME );
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_on_data_received( OnDataReceived );
  a2dp_sink.set_address_validator( ValidateAddress );
  a2dp_sink.set_stream_reader( StreamReader );
  a2dp_sink.start( "ESP32 Sink" );

  Serial.println( "A2DP Sink started..." );

#elif 0

  // Perform Bluetooth classic scan
  Serial.println( "Scanning for BT Classic devices..." );

  BT.begin( "ESP32 Test" );
  BT.discoverAsync( BTAdvertisedDeviceFound );

/*  BTScanResults*  scanResults = BT.discover();

  Serial.println( str( "Discovered %d devices", scanResults != NULL ? scanResults->getCount() : 0 ) );
  if ( scanResults != NULL ) {
    for ( int i=0; i < scanResults->getCount(); i++ ) {
      BTAdvertisedDevice* device = scanResults->getDevice( i );
      Serial.println( str( "  Device %d -> %s (RSSI = %d)", i, device->getName(), device->getRSSI() ) );
    }
  }
*/
/*  // Perform BLE scan
  Serial.println();
  Serial.println( "Scanning for BLE devices..." );

  BLEScan*  scan = BLEDevice::getScan();
  BLEScanResults  scanResults2 = scan->start( 30000 );

  Serial.println( str( "Discovered %d devices", scanResults2.getCount() ) );
  for ( int i=0; i < scanResults2.getCount(); i++ ) {
  	BLEAdvertisedDevice device = scanResults2.getDevice( i );
    Serial.println( str( "  Device %d -> %s (RSSI = %d)", i, device.getName(), device.getRSSI() ) );
  }
*/

#endif

}

void loop() {
  delay( 1000 );  // to prevent watchdog in release > 1.0.6
}

#endif
