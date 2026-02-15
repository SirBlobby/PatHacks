# Hardware

This is the hardware component of our project. It contain a microcontroller, a microphone, a display, and a speakers.
The microcontroller that we are using is the Seeed Studio XIAO ESP32-S3 Sense. The microphone is included on the ESP32-S3 Sense expansion boards. Both the microphone and speaker amp communicate audio data using I2S. The display communicate over SPI.

The microcontroller is using the Arduino framework on platformio. The specific audio amp is the Adafruit MAX98367. Use the default Arduino I2S.h library for the speaker and microphone. The display is an 128x64 monochrome OLED. It uses SPI, NOT I2C. U8g2 will be used for graphical display on the OLED screen.

Create files that deal with each electrical components, one for display, one for speaker, one for microphone. A central test program will run through each component to test and make sure that each items work. Also create a PINOUT.md file that descripe how each comopnent is wired into the microcontroller.

There are links below containing information on the pinout of the microcontroller and other modules.

Links:
https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
https://wiki.seeedstudio.com/xiao_esp32s3_sense_mic/
https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp/pinouts
https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp/arduino-wiring-test-2
https://learn.adafruit.com/monochrome-oled-breakouts/wiring-128x64-oleds
https://www.adafruit.com/product/938



