This 3rd test is here to stabilize my nRF24L01 library and see how fast it can really go!

Up until now, the regular use of the library to transmit packets slowly is working nicely but we definitely need something stronger to do some actual audio streaming!

* I discovered that using the auto-ACK mechanism was actually quite slow and should definitely be disabled
  * This implies monitoring the IRQ and FIFO status flags ourselves
* Using interrupts is also not a great idea



# Issues with ADC

I discovered the analogRead() function takes **a lot** of time and shouldn't be used inside a 32 kHz interrupt!

Check that thread for more info: https://forum.arduino.cc/t/timer1-interrupt-debuging/167256/5

But I found an audio library that somewhat manages the trick of sampling sound at high frequency (!!)...

# nRF24L01 Audio Library from TMRh20

The author apparently manages to send/receive proper audio using the nRF24L01 library (from the same guy).

From what I could see:

* For the receiver :
  * He uses timer 1 at 1/16th the sampling rate so he gets 2 chances of reading a packet at the proper place (which is a double buffer of 32 samples :arrow_right: he writes into a buffer while reading from another (no need for the 256 bytes buffer I'm using)
  * He plays samples using a PWM interrupt, instead of using an external DAC like I'm doing (not sure it's great quality but at least you don't need any extra parts)
* For the transmitter :
  * He uses timer 1 (probably at the same 1/16th sampling rate) to check if a buffer is ready to get sent, then writes the samples to the radio. It supports 32 8-bits samples, or 25 10-bits samples.
  * He uses ADCSRA / ADCSRB / ADMUX voodoo to drive the ADC sampling rate, which I'm going to be using as well because the analogRead() function is too slow to be used inside the interrupt handler!

# ADC Voodoo

Here's what I gathered reading the documentation (chapter 24 of the Atmel document):

###### There are 5 important ADC registers:

* **ADMUX**, used to select reference voltage source, how the result should be stored (either left- or right-adjusted), analog port channel to be used for conversion.

* **ADCSRA**, responsible for enabling ADC, start ADC conversion, prescaler selection and interrupt control.

* **ADCSRB**, responsible for selecting the trigger modes

* **ADCH** and **ADCL**, the data register holding the 10 bits result (either left- or right-adjusted depending on **ADLAR** bit)

  * You can perpetually read from the ADCH register to obtain 8-bits samples

  * You need to read ADCL then ADCH registers to obtains 10-bits samples

###### So basically, we need to:

1. Configure the ADMUX so it reads from input A0
2. Setup ADCSRB to select the "free-running" trigger mode, that will continuously sample the source once we start and enable the ADC
3. Setup ADCSRA to enable and start the ADC using a prescaler value of 32 because we need to achieve at least a 32 kHz sampling rate and a ADC operation takes ~13 cycles so the clock rate must be 16 MHz / 13 / 32 = 38461 Hz. Also, we specify that we want to left-align results so we only need to read the 8 MSbits from ADCH.
4. Read ADCH in the 32 kHz timer and send the samples through the radio...