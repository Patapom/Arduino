So, after I realized Bluetooth is basically hell (main design specs are 3000 pages long, each protocol description is 100 pages long), I turned back on using the MAX98357 I2S mono amplifier that I'll be driving with a 44.1 kHz sample sent via ESP-Now. I ordered some INMP441 I2S microphones as well, to make the communication full duplex. Hopefully, keeping the communication digital until the last minute should reduce the amount of noise to a minimum and I should manage to avoid the difficult analog filtering stage that made me abandon manual amplification in the first place...



# Communication Design

The idea is to use ESP-Now in broadcast mode so packets are not acknowledged and broadcast only once to the general audience, unlike with unidirectional communication between 2 devices where packets are acknowledged and possibly re-sent multiple time.
This yields a better bandwidth but like UDP, there's no guarantee our packets will be received (and indeed, I obtain a rate of ~7% packet loss).



## Audio packets from Central :arrow_right: Peripherals 

The good news is that there's only a single HD stream (used for voice or music) going from the Central to all the Peripherals at once, the audio packets should be 44.1 kHz 16-bits stereo samples = **1.4112 Mbps** bandwidth.

* We have a *Central* device (ID=0), that broadcasts packets to every *Peripheral* devices. Packets have a 2 bytes signature header, 1 byte containing an ID mask destined to one or several peripheral devices (we can have up to 8 devices with ID [1,8]), 1 byte indicating the *[sample slot index](#Common Sample Slot Index)* (in [0,255], loops around), then the audio payload.

* We have up to 8 *Peripheral* devices that will match packets so that the MAC address is the address of the Central, the packet's header must also match, then it will check the ID mask with its own ID to see if it's a target of the packet. The packet is discarded if not, otherwise it's read and appended to its audio stream for replay

## Audio packets from Peripherals :arrow_right: Central

In the other direction, the *Peripherals* could theoretically all stream some voice from their microphones at the same time, so even though we need a lower sampling quality (typically, 16 kHz 16-bits mono samples = **0.256 Mbps** bandwidth), we must accommodate potentially multiple packets from multiple sources as well as handling [echo cancellation](#Echo Cancellation)!

* A *Peripheral* should start sending audio packets as soon as the **volume level** rises above a certain level for a certain time, then it sends the 2 bytes header,1 byte indicating the sample index, then the audio payload (no ID byte necessary as we necessarily target the central device)
* The *Central* will receive potentially multiple payloads, it will check the header then **merge** the payloads destined to the same sample index as well as **OR-ing** the ID of the device sending some audio (obtained using the MAC address of the sender)
  * We could also imagine a version where each *active* peripheral (i.e. with a volume level above a given threshold) would have its own stream sent to the PC (it would help to differentiate peripherals and perform echo cancellation)

The merged audio buffer is then processed for **echo cancellation** and **speech-to-text**... (eventually done on the PC, after being sent via USB, I'm not sure if audio decoding and echo cancellation should even be done on the Central really?)



# First Test

For this first ESP-Now test, I only wanted to send a 16bits stereo WAV sample at 22050 Hz across 2 devices.

The main issue -- I originally thought -- is the high-priority ESP-Now task that shouldn't be reading the SPIFFS WAV file directly, so we need to pre-load some samples into a buffer, then the 22050 Hz timer will query the samples from the buffer before sending them to the ESP-Now.

On the receiver side, the ESP-Now callback should store the audio packet into a buffer that will be replayed by the I2S player which requires 512 samples (**2 KB**) of samples at a time.

## Fixing the "SPIFFS issue"

Turns out it wasn't a SPIFFS issue but more of a "don't do heavy stuff inside a timer IRQ" issue! :smile:

I fixed that by simply indicating to the main loop that the timer ordered us to send a packet! And I started to hear a nice sound going across my devices, save from a clear buffer underrun issue where the player was obviously reading faster than the rate at which our packets could arrive...

## Going bold

I eventually decided to go from 8-bits mono samples at 22050 Hz up to the full 16-bits stereo samples at 44100 Hz!



# ESP-Now Maximum Data Rate

According to this [thread](https://github.com/pschatzmann/arduino-audio-tools/discussions/393), the ESP-Now maximum data rate is not beefy enough to support 44.1 kHz stereo 16-bits sampling rate. They advise to use audio CODECs to perform (de)compression of the audio packets on the fly.

Before we resort to using CODECs though, we need to exhaust all our possibilities and check that WiFi rate configuration function for ESP-Now:

```c
esp_err_t esp_wifi_config_espnow_rate( wifi_interface_t ifx, wifi_phy_rate_t rate )
```

There are quite a lot (!!) of possible WiFi rate values, most notably:

```c
	/**
  * @brief WiFi PHY rate encodings
  *
  */
typedef enum {
    WIFI_PHY_RATE_1M_L      = 0x00, /**< 1 Mbps with long preamble */
    WIFI_PHY_RATE_2M_L      = 0x01, /**< 2 Mbps with long preamble */
    WIFI_PHY_RATE_5M_L      = 0x02, /**< 5.5 Mbps with long preamble */
    WIFI_PHY_RATE_11M_L     = 0x03, /**< 11 Mbps with long preamble */
    WIFI_PHY_RATE_2M_S      = 0x05, /**< 2 Mbps with short preamble */
    WIFI_PHY_RATE_5M_S      = 0x06, /**< 5.5 Mbps with short preamble */
    WIFI_PHY_RATE_11M_S     = 0x07, /**< 11 Mbps with short preamble */
        (...)
};
```

I tried several values:

* 2 Mbps was not fast enough, I was flooding the ESP-Now with too many packets that didn't have time to get sent and I reached the end of the buffer (ESPNOW_NO_MEM error)
* 11 Mbps was much too fast and it didn't bring anything better than the 5.5 Mbps (probably using more power too!)

I finally settled on the **WIFI_PHY_RATE_5M_L** (5.5 Mbps with long preamble).
With it, I did manage to reach 44.1 KHz :tada: without any serious issue other than about **7%** of my packets getting lost on the way! :scream:

Anyway, the ESP-Now is clearly capable of transmitting sound at high quality, which is excellent news! No need for CODECs after all? Well, not so fast!





## CODECs

An interesting note to make is that the rate of lost packets doesn't go down if we use more bandwidth, e.g. with the 11 Mbps WiFi settings.

Then, an interesting test to do is to reduce the amount of packets sent from 723 per second (= 44100 samples) down to 361 per second (22050 samples), the amount of lost packets then goes way down from ~55 (7.6%) down to ~5 (1.4%) !

So that's it! To lose fewer packets, we need to send fewer packets. (Does that mean that in order not to lose any packet, we shouldn't send any? :thinking:)

To send fewer packets, we can't reduce the sample quality or the sampling rate because we really need that 44.1 KHz 16-bits stereo CD quality, so we'll need to compress the samples using CODECs...

A probably good fit would be the [OGG format](https://xiph.org/ogg/)?

### Packets Lost and Distance

I moved my emitter to various parts of the house and witnessed quite a dramatic rise in the rate of lost packets!

* Just moving to the room next door increased packets loss from 1.7% to 5%!
* And moving to the basement increased to almost **80% of packets lost**!

Will I need to insert special **repeater devices** in the mesh? Why not starting to use the house's WiFi network then? We'll see...



## Sleep Mode

Continuous transmission at 44.1 KHz makes the WiFi module on the transmitting ESP32 quite hot (probably around 40°C) so we need to setup a On/Off switch to kill the transmission of packets when no packets are available.

Thankfully, it's easy for the Central to know when to turn On or Off the packets since it decides what and when to send.

For a Peripheral, it basically depends on the [volume level](#Volume Level) of the microphone... (and even though, its transmission rate is lower than the Central's, so it should be less affected by the heating issue)



# Common Sample Slot Index

The sample slot index in the audio packets should all reference some sort of universal index in the peripheral/central device's audio buffer corresponding to a **slot** containing a certain amount of samples, for example a packet-worth of samples, e.g. floor( 246 bytes / 4 bytes per sample ) = 61 samples = 1.383 millisecond of audio.

Ideally, it should be computed using a clock synchronized between devices (check the **time()** and gettimeofday() functions that return µs precision), devices could exchange packets once a day (or depending on discrepancies between clocks) to determine their offset compared to the central device.

**PROBLEM** ==> We only read our own time when we receive the packet, this totally prevents us from any synchronization effort!



A peripheral would get its time value using:

```c++
	long	myTime_µs = gettimeofday();
			myTime_µs += differenceWithCentral;	// Computed from the time sent by the central? But how?
```

Then a sample slot index would be some integer measure of time:

```c++
	U8	sampleSlotIndex = U8( myTime_µs / 1383 );	// Each slot is 61 samples = 1.383 millisecond
```

The devices will then insert the received audio packet at the proper slot in their replay buffer, and fill the missed slots with 0 (e.g. we receive data for slot 0, 1, 2 then 4, we must fill slot 3 with zeroes before filling slot 4).

## What to do when we miss a slot (i.e. packets are lost)?

What should we do when we miss a packet?

* I tried filling the space with 0 but it introduces some kind of horrible artefacts
* Same when I duplicate the same packet several times

* Should we **expand** the current packet across several slots?
* Or should we **interpolate** between the last packet's last value and the new packet's first value?



# Volume Level

We can determine the volume inside a given audio packet by simply summing the square of the sample values:
$$
V = \frac{1}{N} \sum_{i=0}^N{f(i)^2}
$$
Then we enable the packet transmission if it goes above a certain volume threshold for enough time/packets.

In the same manner, we disable the packet transmission if the volume level goes below a threshold for enough time/packets.



# Echo Cancellation

Each peripheral will inevitably send back many echoes of the sound output by their speakers, each echo being either directly perceived by the microphone (so probably with a fixed delay that needs to be determined), or reverberated multiple times by the environment depending on the configuration of the room and the materials used there.

* A good thing is that the expensive echo cancellation will be done on the PC, where a lot of firepower is available.

* We assume a single echo is some version of the sample that was sent by the PC with some delay and some noise / frequency shifting.
* The cancellation will be facilitated if the peripheral streams are separated instead of merged.
  * The merged version will complicate things because we'll have to deal with all the echo patterns from all the devices, although the algorithm should be the same: remove altered copies of our own stream
  * The separated streams will have a single pattern of echoes that shouldn't vary much, and we could use the device's ID to quickly identify and process the echoes for a particular device
* Each device has a probably fixed set of echoes, unless it moves inside the room a lot.
  * We could use an echo cancellation calibration routine that plays some continuous tones at various frequencies so we can quantify and qualify the echo response for a particular device



