## Install and configure Arduino IDE

Make sure you use a version of the Arduino IDE which is supported by the ESP8266 platform. Follow the [tutorial on our documentation platform][Tutorial].

## Install libraries in Arduino IDE

Install the following libraries with your Arduino Library Manager in `Sketch` > `Include Library` > `Manage Libraries...`
* ESP8266 Weather Station
* JSON Streaming Parser by Daniel Eichhorn
* ESP8266 OLED Driver for SSD1306 display by Daniel Eichhorn. **Use Version 3.0.0 or higher!**

## Prepare the software
* [Create an API Key][API Key] for OpenWeatherMap
* In the Arduino IDE go to `File` > `Examples` > `ESP8266 Weather Station` > `Weather Station Demo`
* Enter the OpenWeatherMap API Key
* Enter your WiFi credentials
* Adjust the location according to OpenWeatherMap API, e.g. Zurich, CH
* Adjust UTC offset

## Setup for PlatformIO

If you are using the PlatformIO environment for building

* choose one of the available IDE integration or the Atom based IDE
* install libraries 561, 562 and 563 with "platformio lib install"
* adapt the [WeatherStationDemo.ino][Example] file to your needs (see details above)
