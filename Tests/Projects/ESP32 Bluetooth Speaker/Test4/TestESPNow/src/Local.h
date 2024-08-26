#pragma once

// On which wifi channel (1-11) should ESP-Now transmit? The default ESP-Now channel on ESP32 is channel 1
#define ESP_NOW_WIFI_CHANNEL 8

//#define CENTRAL_TO_PERIPHERAL_RATE	44100	// High-quality audio (44.1KHz 16-bits stereo)
#define CENTRAL_TO_PERIPHERAL_RATE	22050		// Medium-quality audio (22050Hz 16-bits stereo)

#define	PERIPHERAL_TO_CENTRAL_RATE	16000	// Microphone at 16KHz is enough for voice quality (human voice only reaches up to 8KHz)
