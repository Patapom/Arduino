# GPS Tracker

There are 2 types of modules designed here:

* Station module, that does the tracking + logging + replaying of GPS info
* Tracker module, that simply broadcasts the GPS info relative to the base via LORA



# Tracker Module

It's comprised of:

* 1x BMS 2S USB-C 5V connected to 2 18650 2500 mAh Li-ion batteries
* 1x Neo-6M GPS Module (+5V + UART communication on Serial1)
* 1x Reyax RYLR998 LORA Communication module (+3.3V + UART communication on Serial2)
* 1x Buzzer that can be activated when LORA receives a command
* (optional) 1x ST7789 TFT Screen (+5V backlight / 3.3V I2C communication) for diagnostics



## Goals

The idea is to broadcast the GPS location via LORA to any 



# Base Module

It's comprised of:

* 1x BMS 2S USB-C 5V connected to 2 18650 2500 mAh Li-ion batteries
* 1x Neo-6M GPS Module (+5V + UART communication on Serial1)
* 1x Reyax RYLR998 LORA Communication module (+3.3V + UART communication on Serial2)
* 1x Buzzer that can be activated when LORA receives a command
* (optional) 1x ST7789 TFT Screen (+5V backlight / 3.3V I2C communication) for diagnostics