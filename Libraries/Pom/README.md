## Pom Library

This is a Visual Studio project containing a new library with advanced helpers.

It contains:

* Advanced math helpers like
** log2
** Values clamping

* PulseTrain helper that records a series of LOW-state pulses until a HIGH-state pulse of a given length is encountered, in which case the packet is deemed finished and the routine returns
 It behaves pretty much like the well-known pulseIn() except it records multiple pulses before returning.

