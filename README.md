# Build

* Stable: [![Build Status](https://api.travis-ci.org/squix78/esp8266-weather-station.svg?branch=master)](https://travis-ci.org/squix78/esp8266-weather-station)
* Development: [![Build Status](https://api.travis-ci.org/squix78/esp8266-weather-station.svg?branch=development)](https://travis-ci.org/squix78/esp8266-weather-station)

# esp8266-weather-station
<img src="https://github.com/squix78/esp8266-weather-station/raw/master/resources/CurrentWeatherReflection.jpg" width="250"/>

Code for the ESP8266 WeatherStation project. You can by a starter kit from here and support the developement:
https://blog.squix.org/product/weatherstation-kit-w-white-oled

## Arduino IDE

Make sure you use a version of the Arduino IDE which is supported by the ESP8266 platform. You can find it here: https://www.arduino.cc/en/Main/OldSoftwareReleases

## Setup Arduino IDE

* Install the following libraries with your Arduino Library Manager in Sketch > Include Library > Manage Libraries...

 * ESP8266 Weather Station
 * Json Streaming Parser (by Daniel Eichhorn)
 * ESP8266 Oled Driver for SSD1306 display (by me as well). **Make sure that you use Version 3.0.0 or bigger!**
* Go to http://wunderground.com, create an account and get an API Key
* In the Arduino IDE go to File > Examples > ESP8266 Weather Station
 * Enter  the Wunderground API Key
 * Enter your Wifi credentials
 * Adjust the location according to Wunderground API, e.g. Zurich, CH
 * Adjust UTC offset

## Setup for PlatformIO

If you are using the PlatformIO environment for building
 * choose one of the available IDE integration or the Atom based IDE
 * install libraries 561, 562 and 563 with "platformio lib install"
 * adapt the WeatherStationDemo.ino file to your needs (see details above)


## Upgrade

The ESP8266 Oled Library changed a lot with the latest release of version 3.0.0. We fixed many bugs and improved performance and changed the API a little bit. This means that you might have to adapt your Weather Station Code if you created it using the older 2.x.x version of the library. Either compare your code to the updated WeatherStationDemo or read through the Upgrade Guide here: [Upgrade Guide](https://github.com/squix78/esp8266-oled-ssd1306/blob/master/UPGRADE-3.0.md)

## Available Modules
* **TimeClient**: simple class which uses the header date and time to set the clock
* **NTPClient**: a NTP based time class written by Fabrice Weinberg
* **WundergroundClient**: fetches current weather and forecast from wunderground.com
* **ThingspeakClient**: fetches data from Thingspeak which you might have collected with another sensor node and posted there. I use this to measure outdoor temperature and humidity and show it on the WeatherStation with a ClimateNode: https://shop.squix.ch/index.php/esp8266/climatenode.html  

## Why Weather Station as a library?

I realized that more and more the Weather Station was becoming a general framework for displaying data over WiFi to one of these pretty displays. But everyone would have different ways or sources for data and having the important part of the library would rather be the classes which fetch the data then the main class.
So if you write data fetchers which might be of interest to others please contact me to integrate them here or offer your code as extension library yourself and call it something like esp8266-weather-station-<yourservice>.
I will gladly list it here as third party library...
