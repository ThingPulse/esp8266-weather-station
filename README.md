# ThingPulse ESP8266 Weather Station


[![ThingPulse logo](https://thingpulse.com/assets/ThingPulse-w300.svg)](https://thingpulse.com)


This code works best with the NodeMCU V2 ESP8266 module and an 0.96" OLED display.
To get you up and running in no time we created a kit which contains all the necessary parts:
[https://thingpulse.com/product/esp8266-iot-electronics-starter-kit-weatherstation-planespotter-worldclock/](https://thingpulse.com/product/esp8266-iot-electronics-starter-kit-weatherstation-planespotter-worldclock/)

By buying this and [other kits](https://thingpulse.com/shop/) from us you are funding maintenance and  development of this library. Thank you!

[![ThingPulse ESP8266 WeatherStation Classic Kit](resources/ThingPulse-ESP8266-Weather-Station.jpeg)](https://thingpulse.com/product/esp8266-iot-electronics-starter-kit-weatherstation-planespotter-worldclock/)

### Build Status

| Branch  | Status |
| ------------- | ------------- |
| master  | [![Build Status](https://travis-ci.org/ThingPulse/esp8266-weather-station.svg?branch=master)](https://travis-ci.org/ThingPulse/esp8266-weather-station)  |
| development  | [![Build Status](https://travis-ci.org/ThingPulse/esp8266-weather-station.svg?branch=development)](https://travis-ci.org/ThingPulse/esp8266-weather-station)  |

## Service level promise

<table><tr><td><img src="https://thingpulse.com/assets/ThingPulse-open-source-prime.png" width="150">
</td><td>This is a ThingPulse <em>prime</em> project. See our <a href="https://thingpulse.com/about/open-source-commitment/">open-source commitment declaration</a> for what this means.</td></tr></table>

## Install and configure Arduino IDE

Make sure you use a version of the Arduino IDE which is supported by the ESP8266 platform. Follow the [tutorial on our documentation platform](https://docs.thingpulse.com/how-tos/Arduino-IDE-for-ESP8266/).

## Install libraries in Arduino IDE

* Install the following libraries with your Arduino Library Manager in `Sketch` > `Include Library` > `Manage Libraries...`
* ESP8266 Weather Station
* JSON Streaming Parser by Daniel Eichhorn
* ESP8266 OLED Driver for SSD1306 display by Daniel Eichhorn. **Use Version 3.0.0 or higher!**

## Prepare the software
* [Create an API Key](https://docs.thingpulse.com/how-tos/openweathermap-key/) for OpenWeatherMap
* In the Arduino IDE go to `File` > `Examples` > `ESP8266 Weather Station` > `Weather Station Demo`
* Enter the OpenWeatherMap API Key
* Enter your WiFi credentials
* Adjust the location according to OpenWeatherMap API, e.g. Zurich, CH
* Adjust UTC offset

## Setup for PlatformIO

If you are using the PlatformIO environment for building

* choose one of the available IDE integration or the Atom based IDE
* install libraries 561, 562 and 563 with "platformio lib install"
* adapt the [WeatherStationDemo.ino](examples/WeatherStationDemo/WeatherStationDemo.ino) file to your needs (see details above)


## Available Modules
* **Time Client**: simple class which uses the header date and time to set the clock
* **NTP Client**: a NTP based time class written by Fabrice Weinberg
* **OpenWeatherMap Client**: A REST client for the OpenWeatherMap.com service, providing weather information
* **Aeris Client**: Client for the service provided by aerisweather.com. Fully functional initial version. After the Wunderground incident (see [upgrade notes](#upgrade-notes)) we first targeted Aeris before we settled with OpenWeatherMap. This code is unmaintained but will remain part of this library for the time being.
* **Wunderground Client - DEPRECATED**: fetches current weather and forecast from wunderground.com. Please note: Wunderground stopped their free tier service and previous demos are now using the OpenWeatherMap client.
* **Thingspeak Client**: fetches data from Thingspeak which you might have collected with another sensor node and posted there.
* **Astronomy**: algorithms to calculate current lunar phase and illumination.
* **SunMoonCalc**: a calculator for sun and moon properties for a given date & time and location. This implementation is port of a [Java class by T. Alonso Albi](http://conga.oan.es/~alonso/doku.php?id=blog:sun_moon_position) from OAN (Spain).

## Why Weather Station as a library?

I realized that more and more the Weather Station was becoming a general framework for displaying data over WiFi to one of these pretty displays. But everyone would have different ways or sources for data and having the important part of the library would rather be the classes which fetch the data then the main class.
So if you write data fetchers which might be of interest to others please contact me to integrate them here or offer your code as extension library yourself and call it something like esp8266-weather-station-<yourservice>.
We will gladly list it here as third party library...

## Upgrade Notes

**Replace Wunderground with OpenWeatherMap as weather data provider**

The weather information provider we used so far (Wunderground) [recently stopped their free tier](https://thingpulse.com/weather-underground-no-longer-providing-free-api-keys/) without previous notice on May 15, 2018. This release adds support for a new provider with a free tier for weather information: OpenWeatherMap.com. The basic demo (WeatherStationDemo) has been adapted to use this new API through the OpenWeatherMapCurrent and OpenWeatherMapForecast REST clients.

Sadly OpenWeatherMap provides less information than Wunderground did (or still does). If you are missing attributes in the response documents then please [contact the OpenWeatherMap team](https://openweathermap.desk.com/customer/portal/emails/new).

**ESP8266 OLED Library upgrade**

The ESP8266 OLED Library changed a lot with the latest release of version 3.0.0. We fixed many bugs and improved performance and changed the API a little bit. This means that you might have to adapt your Weather Station Code if you created it using the older 2.x.x version of the library. Either compare your code to the updated WeatherStationDemo or read through the [upgrade guide](https://github.com/ThingPulse/esp8266-oled-ssd1306/blob/master/UPGRADE-3.0.md)

## Deprecation notes

| Announcement  | Module  | Removal  |
|---------------|---------|----------|
| 2018-06-13    | all **Wunderground** related code, see [our blog](https://thingpulse.com/hello-openweathermap-bye-bye-wunderground/) for details  | June 2019  |
