# ESPboy EBT Tracker
[by shiru8bit](https://shiru.untergrund.net) (shiru@mail.ru) 

(WTFPL license)[https://ru.wikipedia.org/wiki/WTFPL]

**[User Manual](https://github.com/ESPboy-edu/ESPboy_EBT_Tracker/blob/main/EBT%20-%20ESPboy%20tracker.pdf?raw=true)**

# About

EBT is a tiny, minimalistic, yet quite powerful chiptune-like music editor with a tracker interface, originally developed for the [ESPboy portable DIY](www.espboy.com) handheld platform.
The source code is portable, so it may be available on other platforms as well. Currently it features an SDL port, so it can run on a regular desktop PC as well, sharing the same data format, so the song and instrument files are easily transferable between the devices.


![ESPboy2 EBT Tracker pic](https://github.com/ESPboy-edu/ESPboy_EBT_Tracker/blob/main/ESPboy2_EBT_Tracker_look.png?raw=true)

# Outputs

A variety of the sound output options. The EBT now supports the built-in speaker via sigma-delta modulation, an external mono via SDM, a stereo PWM (was it ever done on the ESP8266?), and stereo via i2s DAC module. [Check I2S DAC module connection schematic here].(https://github.com/ESPboy-edu/ESPboy_PT3Play)

![ESPboy2 EBT Tracker pic](https://github.com/ESPboy-edu/ESPboy_EBT_Tracker/blob/main/ebttrackeroutputs.png?raw=true)

# How to compile
  - Install Arduino IDE https://www.arduino.cc/en/Main/Software
  - Install the WeMos D1 mini ESP866 board to your Arduino IDE https://www.instructables.com/Wemos-ESP8266-Getting-Started-Guide-Wemos-101/
  - Install all the libs for ESPboy from here https://github.com/ESPboy-edu/ESPboy_ArduinoIDE_Libraries
  - Select the "LOLIN(WEMOS) D1 R2 & mini" board in the Arduino IDE and compile this code


**IMPORTANT!**
  if you are using/updating original TFTeSPI library
  you have to edit or replace usersetup.h file in the TFTeSPI folder in your Arduino libraries collection
  according to this https://github.com/ESPboy-edu/ESPboy_ArduinoIDE_Libraries/blob/master/TFT_eSPI-master/User_Setup.h

