////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Music Receiver through Bluetooth classic & BLE
//  => BAsically, P. Schatzman's code, just to test if it's working...
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#if 0

#include <Arduino.h>
#include "Global.h"

#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

char* str::ms_globalBuffer = s_globalBuffer;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

I2SStream           i2s;
BluetoothA2DPSink   a2dp_sink( i2s );
BLECharacteristic*  pCharacteristic;
char                title[160] = {"Undefined"};

void avrc_metadata_callback( uint8_t id, const uint8_t* text ) {
  Serial.printf( "==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text );
  if ( id == ESP_AVRC_MD_ATTR_TITLE ) { // Title of the playing track
    strncpy( title, (const char*) text, 160 );
    pCharacteristic->setValue( title );
  }
}

//hw_timer_t* timer0Config = NULL;

//void IRAM_ATTR  Timer0_ISR();

void setup() {
  pinMode( PIN_LED_RED, OUTPUT );

  Serial.begin( 115200 );
  while ( !Serial );            // Wait for serial port to connect. Needed for Native USB only

  Serial.println();
  Serial.println( str( "Clock Frequency = %d.%03d MHz", U16( F_CPU / 1000000L ), U16( (F_CPU / 1000L) % 1000UL ) ) );



/*  timer0Config = timerBegin( 32000 ); // 32KHz
  timerAttachInterrupt( timer0Config, &Timer0_ISR );
//  timerAlarm( timer0Config, 50000, true, reload count?? );
  timerAlarm( timer0Config, 1, true, 0 ); // Signal every time counter reaches 1, i.e. use the full 32KHz frequency of the timer...
  timerRestart( timer0Config );

  Serial.println( str( "Timer enabled..." ) );
//*/


  // start a2dp in ESP_BT_MODE_BTDM mode (Dual mode = supports both Bluetooth Classic and Bluetooth Low-Energy)
  a2dp_sink.set_default_bt_mode(ESP_BT_MODE_BTDM);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.start("MyMusic");
//  a2dp_sink.start("EMEET OfficeCore M0 Plus");
  Serial.println("A2DP Started!");

  // start BLE
  BLEDevice::init("MyMusic");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService( SERVICE_UUID );
  pCharacteristic = pService->createCharacteristic( CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ );

  pCharacteristic->setValue(title);
  pService->start();
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID( SERVICE_UUID );
  pAdvertising->setScanResponse( true );
  pAdvertising->setMinPreferred( 0x06 ); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred( 0x12 );
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

char* debugText = NULL;

bool  prout = true;
void loop() {

  digitalWrite( PIN_LED_RED, prout );
  prout = !prout;
  delay( 500 );

  if ( debugText != NULL ) {
    Serial.println( debugText );
    debugText = NULL;
  }
}
/*
// --------------------------------------------------------
// Timer IRQ
//
void IRAM_ATTR  Timer0_ISR() {
//  U8  sampleIndex8Bits = U8( sampleIndex >> 8 );
//  currentSample = samples[sampleIndex8Bits];  // ESP32 DAC expects 8-bits samples...
//  sampleIndex += sampleInc;


//  dacWrite( PIN_DAC1, currentSample );

// DON'T!! Causes guru meditation + reboot
//  Serial.print( "+" );
}
*/

#endif