unsigned long pulseInSimpl(volatile unsigned char *port, unsigned char bit, unsigned char stateMask, unsigned long maxloops) {
    unsigned long width = 0;
    // wait for any previous pulse to end
    while ((*port & bit) == stateMask)
        if (--maxloops == 0)
            return 0;

    // wait for the pulse to start
    while ((*port & bit) != stateMask)
        if (--maxloops == 0)
            return 0;

    // wait for the pulse to stop
    while ((*port & bit) == stateMask) {
        if (++width == maxloops)
            return 0;
    }
    return width;
}
