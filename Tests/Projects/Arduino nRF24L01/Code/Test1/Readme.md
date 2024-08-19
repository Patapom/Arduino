This second test is here to see how fast we can transmit data across the devices.

This time, on top of the previous test's connections, we're using a DAC module and an audio amplifier to play some soundwaves at 16KHz by reading from a circular buffer that is fed regularly with data packets received from the transmitter. This is getting **SERIOUS** ! :grin:



# Mic Amplifier (Input)

I'm using the **MAX9814** Electret Microphone Amplifier which is quite straightforward to connect:

![Mic Amplifier](I:\Bootstrap\Projects\Arduino nRF24L01\Code\Test1\Mic Amplifier.PNG)

* In my case, I connected the Vdd (V+ in the image) to 5V instead of 3.3V.
* Gain is left unconnected, yielding a 60dBm gain (50dBM when connected to ground, 40dBm when connected to Vdd).
* A/R means Attack/Release ratio and is left unconnected, which means a 1:4000 ratio. If connected to Vdd we get a 1:2000 ratio, and if connected to GND we get a 1:500 ratio. Other, more precise, ratios can be obtained by connecting specific capacitors (cf. Table 2 in the documentation).
  It's basically used to mute static when no sound is present, then start an **attack** mode when a sound occurs, then a **sustain** mode while the sound level is high enough, then finally a **release** mode that slowly brings the amplifier down to its lowest gain when the sound level is too low.



It's connected on the Arduino R3 (transmitter module) that samples the mic at 16KHz and stores the 8 bits samples into a buffer.

When 32 samples (32 bytes) are available, they're transmitted via the nRF24L01 to the receiver module on the Nano board.



# Sound Amplifier (out)

I'm using a **PAM8610** Stereo Audio Amplifier 2x10W (CLASS-D AUDIO POWER AMPLIFIER WITH DC VOLUME CONTROL).
It needs a 12V DC - 1A power supply.

<img src="I:\Bootstrap\Projects\Arduino nRF24L01\Code\Test1\PAM8610.jpg" alt="PAM8610" style="zoom:50%;" />

## Signal

The samples are received via the nRF24L01 RX_DR interrupt and stored into a small ring buffer via 32 bytes packets.

The buffer is then read in a 16KHz interrupt routine and transformed into an analog signal via a Digital to Analog Converter (DAC) that will feed the audio amplifier.

# Feeding the amplifier

We need to feed the amplifier with an analog signal but unfortunately, the Nano (or even Uno) boards don't offer these functionalities. Only "PWM analog", which is not suitable. **Idea** :arrow_right: Any way we can transform a PWM into a voltage?

So we're gonna need an external DAC chip to do the job. I studied several possibilities.

## 12-bits DAC (MCP4725)

Unfortunately, this tiny DAC uses I2C for communication, which is super slow!

According to the official documentation, I2C wires SDA and SCL should be connected to A4 and A5 respectively:

![Arduino Nano v3 Pin Layout](I:\Bootstrap\Projects\Arduino nRF24L01\Code\Test1\Arduino Nano v3 Pin Layout.png)

* I2C trop lent !
* Pas de **I2S (I2C, but for sound)** sur Nano (seulement sur des boards spéciales genre "Zero")
* R-2R ladder mais avec MUX et latching pour éviter de bouffer 1000 pins de l'Arduino ???



## 16-Bits DAC (DAC8560)

C'est du lourd ! Et malheureusement, ça demande un power supply de minimum 10V pour le côté analogique... Ca commence à faire beaucoup, avec l'ampli qui veut aussi du 12V !

* Se pilote en SPI (mais MODE1, donc va falloir changer de mode à la volée)

* Commandé 3 chez Mouser le 4 juillet...



## TLC 7528

This is an 8-bits parallel DAC, so it's using **a lot** of pins on the board! Do we want that?

**@TODO** Try it...

## R-2R ladder

A simple R-2R ladder scheme could be used to drive voltage, as in [this project](https://www.instructables.com/Arduino-Audio-Output/) by Amanda Ghassaei.



# ESP32 Version

Frustrated by the slow receiver response on the Nano (and the lack of DAC), I decided to port the code to ESP32 which was quite easy to do, thanks to the Arduino library existing on that microcontroller.

Unfortunately, I basically had the same results: the emitter sending ~4 Hz LED pulses and the receiver sending ~2 Hz pulses, as if we received only half the packets we sent, which could be verified on the oscilloscope.

At least, I learned to use the ESP32... (I'm a bit skeptical about that beast, especially the huge compilation time compared to the Arduino, and the annoying fact that you have to press a button to upload and flash your code!)
