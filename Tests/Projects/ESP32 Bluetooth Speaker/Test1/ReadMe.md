**CURRENT STATUS:**  I find the Bluetooth protocol a horrible mess and the A2DP protocol difficult to play with, even the basic example from Phil Schatzmann's library doesn't really work because of a delay( 1 ) **inside** the A2DP callback that makes the replay stutter, and if I remove it then the watchdog event occurs and reboots the device because other tasks are starved!
Finally, by playing with the MAX98357, I realized the sound quality was actually very good and decided to go back to my original idea of using an amplifier and mic on the ESP32 board and send it to the PC... Cf. Test2 for new adventures!



The idea with this first test is to establish a Bluetooth connection with a teleconference speaker and microphone called the "EMEET M0 plus", and to send and receive audio through that device and, in the future, send it to the PC server that will do the voice processing and text to speech.

I already have a kind of "proof of concept" that I could have this kind of full duplex transfer work with Arduinos (cf. the nRF24L01 project) but the quality is flimsy at best, and I would still have to deal with a *huge* amount of work about sound filtering and mic amplification issues that, frankly, I don't give a damn about!

So why bother with such complicated matters when there already exist plenty of devices that do a very nice job of sending/receiving audio with good quality?



# EMEET OfficeCore M0 Plus

Product page: https://emeet.com/en-ca/products/speakerphone-m0-plus

![EMeet M0 Plus](I:\Bootstrap\Projects\ESP32 Bluetooth Speaker\Test1\EMeet M0 Plus.png)

Features 4-mic array to cover 360° voice pickup.

Bluetooth Range: Up to 10m (33ft)

Supported Bluetooth Profiles: **A2DP**, AVRCP, HFP Profiles, SBC codec



# A2DP Library

Library page: https://github.com/pschatzmann/ESP32-A2DP

Works with another library from the same guy, the Arduino Audio Tools library: https://github.com/pschatzmann/arduino-audio-tools

He explains how to send sound to a Bluetooth speaker here: https://www.pschatzmann.ch/home/2020/09/15/sending-sound-from-an-esp32-to-a-bluetooth-sink-e-g-bluetooth-speaker/



## Handling large applications

Unfortunately, when compiling with the A2DP or the audiotools library, we get a compiler error telling us the application is too large for the default 1.2MB partition size.
The fix is easy enough and consists into specifying the partition scheme using a CSV file:

​	`board_build.partitions = huge_app.csv`

## Uploading data to the ESP32

I also decided to upload a WAV file to the board for sound testing purposes. This is also quite simple using the PlatformIO development tools (source: https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/).

We simply need to:

1.  Create a **data** folder in the root project folder

2. Move our data files in there

3. Run the **Build Filesystem Image** task (platformio.exe run --target buildfs --environment esp32dev)
   For me, I got these lines during task execution indicating this was working:
      *Bu*ilding in release mode
      *Building FS image from 'data' directory to .pio\build\esp32dev\spiffs.bin*
      /Alarm03.wav

   

4. Run the **Upload Filesystem Image** task (platformio.exe run --target uploadfs --environment esp32dev)

   1. I first had a fatal error (A fatal error occurred: Could not open COM10, the port doesn't exist*** [uploadfs] Error 2) but that's because I was monitoring the running program and the COM10 port was busy
   2. After killing the monitoring task, I had a successful upload...
      

5. To use the SPIFFS file system and access our file, we simply need this code:

```c++
#include "SPIFFS.h"

if ( !SPIFFS.begin( false ) ){
    Serial.println( "An Error has occurred while mounting SPIFFS" );
    return;
}

File file = SPIFFS.open( "/text.txt" ); // open() returns a std::shared_ptr<FileImpl>
if ( !file ){
    Serial.println("Failed to open file for reading");
    return;
}

Serial.println( "File Content:" );
while( file.available() ){
    Serial.write( file.read() );
}
file.close();
```



# Principle

I'm relying on the Bluetooth standard and some libraries to do the heavy work of transporting sound to and from a PC, but the main issue is that the Bluetooth standard is just hellish to understand and there's no actual documentation on how to use the EMEET in both ways at the same time.

Basically what we're trying to achieve is to emulate what Windows is doing: connect to the device as an output speaker and an input microphone.

## Making the devices available for discovery

First, there's the connection issue: as with all Bluetooth devices, once it's paired and connected to something else like a PC or a Smartphone, the device won't let go and we can't connect with the library running on the ESP32. So first, we have to make sure that Windows or my Samsung Galaxy don't "hog" the device!

Once the device is ready to be paired, we can press the Bluetooth icon for a short while, a chime should be emitted and a blue LED should start blinking indicating the device is waiting to connect.

## Connecting to the device as a Source (i.e. ESP32 is the sound emitter)

We can declare a "Source" object:

​	`BluetoothA2DPSource a2dp_source;`

Then by providing a SSID callback, we can tell whether we want to connect to the found devices or not.

Once connected, we provide a callback that will feed "data frames" (a Frame is 2 16-bits channel values, declared in SoundData.h).

## Connecting to the device as a Sink (i.e. ESP32 is the sound receiver)

At the moment, I haven't found a way to connect to the EMEET's microphone acting as a Source.

## Connecting to the device both as a Source and a Sink

I'm trying to decipher the incredibly complex Bluetooth and A2DP standards to understand how it's possible.

There must be something I can do to configure the A2DP to run both ways but I'll probably have to tweak the A2DP library...



# Switching to VSCode and PlatformIO

I had a very bad experience using Arduino IDE and ESP32 development so far: super long compilation times, recompiling everything even though the code didn't change, having to be there and have the finger on the "upload" button on the ESP32 board otherwise you have to upload (and then compile) again!
Moreover, after using the audio-tools library and compiling for at least 5 minutes (!!), I had the unpleasant message that my compiled library overflowed the 1310720 kB size!



So I decided to switch to VSCode. I'm a little familiar with it already and it's also available on Linux which will facilitate any transition...
I also decided to install the Arduino / ESP32 / etc. development platform "PlatformIO" which is quite simple to use from within VS Code.

I was a bit put off by the fact that you **have to** import/convert your project first, which creates a new random folder in your User folder, but once I moved that folder back to where my original project was, everything started to compile and work!

Compilation time is "ok" and at least you don't have to recompile the entire planet if nothing changed. Also, you can *just upload* your library if it's ready and no changes in the code occurred!!!

I was also a bit disappointed to see that every library import had to go through their library manager that needs their server to work, so no more "import library from ZIP file" option!

But a quick search (source: https://community.platformio.org/t/how-integrate-external-personal-zip-libraries/17407/2) gave me the solution:

1.  Open a PlatformIO CLI terminal
2. Type in **pio lib -g install "C:\Users\Max\Downloads\Adafruit-GFX-Library-master.zip"**

**UNFORTUNATELY**, I had the same message about my library overflowing the available program space of the board!

**FORTUNATELY**, after another search (https://community.platformio.org/t/how-to-optimize-code-for-flash-memory-esp32-arduino-framework/26609/3), I found the solution that consists in putting this line inside the **platformio.ini** file:

​	**board_build.partitions = huge_app.csv**

And the compiled library went from 140% down to 43% of available memory. Just like that...