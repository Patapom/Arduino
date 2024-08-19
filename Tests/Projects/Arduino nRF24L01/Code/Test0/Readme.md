This simple test allows to transmit individual packets at 2Mbps but at a peaceful pace: 1 packet every second.
The R3 is the transmitter, the Nano is the receiver.

Connections with the board should look something like this:

![Connections](I:\Bootstrap\Projects\Arduino nRF24L01\Code\Test0\Connections.png)

**WARNING**  Check Global.h for which pins are actually used for CE and CSN, they're not the same as in the image!

I wasn't supposed to use IRQs but I did anyway, in the Nano part, which makes the code even simpler: you just have to read the payload in the IRQ handler and clear the RX_DR IRQ bit in the status register! No need to play with the Enable/Disable states: always leave the chip enabled!

For reference, here are the nRF24L01 module's connection pins:

![nRF24L01 Pins](I:\Bootstrap\Projects\Arduino nRF24L01\Code\Test0\nRF24L01 Pins.png)