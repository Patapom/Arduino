# GPS Tracker

There are 2 types of modules designed here:

* Station module, that does the tracking + logging + replaying of GPS info
* Tracker module, that simply broadcasts the GPS info relative to the base via LORA



# Tracker Module

It's comprised of:

* 1x ON/OFF switch
* 1x BMS 2S USB-C 5V connected to 2 18650 2500 mAh Li-ion batteries
* 1x Neo-6M GPS Module (+5V + UART communication on Serial1)
* 1x Reyax RYLR998 LORA Communication module (+3.3V + UART communication on Serial2)
* 1x Buzzer that can be activated when LORA receives a command
* (optional) 1x ST7789 TFT Screen (+5V backlight / 3.3V I2C communication) for diagnostics



## Goals

The idea is to broadcast the GPS location via LORA to any listening device using LORA network ID 6 on 915 MHz.
I'm expecting long distance 433MHz modules soon so I may switch to those when I have them since they're more robust to forest and terrains with obstacles!

The LORA can also receive some commands, the only one supported for now is used to start/stop the buzzer. This will be used to locate the device by sound when the Station Module is close enough (less than 100 meters).



# Station Module

It's comprised of:

* 1x ON/OFF switch
* 1x BMS 2S USB-C 5V connected to 2 18650 2500 mAh Li-ion batteries
* 1x Neo-6M GPS Module (+5V + UART communication on Serial1)
* 1x SD Card Reader (+3.3V + I2C communication) for storing & replaying GPS path data
* 1x Reyax RYLR998 LORA Communication module (+3.3V + UART communication on Serial2)
* 1x ST7789 TFT Screen (+5V backlight / 3.3V I2C communication) for menu & diagnostics
* 4x small buttons for menu navigation
* 4x large colored buttons for bookmarking



## Goals

It's a multi-function module that can be used in:

1. Explore mode, stores a GPS path with possible points of interest (PoI) indicated by pressing the large colored buttons
2. Replay mode, reads back a specific GPS path and indicates the direction/distance of the nearest PoI
3. Tracking mode, actively tracks one or multiple Tracker Modules broadcasting their GPS position, or a manually entered position

