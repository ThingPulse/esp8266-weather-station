# esp8266-weather-station

New version of the ESP8266 Weather Station

## Arduino IDE

Make sure you use a version of the Arduino IDE which is supported by the ESP8266 platform, e.g. 1.6.5. You can find it here: https://www.arduino.cc/en/Main/OldSoftwareReleases#previous

At the time of this writing **1.6.6 and 1.6.7 are not supported**!

## Setup

* Install the following librarys with your Arduino Library Manager in Sketch > Include Library > Manage Libraries...
 * ESP8266 Weather Station 
 * Json Streaming Parser (by Daniel Eichhorn)
 * ESP8266 Oled Driver for SSD1306 display (by me as well). **Make sure that you use Version 2.0.0 or bigger!**
* Go to http://wunderground.com, create an account and get an API Key
* In the Arduino IDE go to File > Examples > ESP8266 Weather Station
 * Enter  the Wunderground API Key
 * Enter your Wifi credentials
 * Adjust the location according to Wunderground API, e.g. Zurich, CH
 * Adjust UTC offset

## Available Modules
* **TimeClient**: simple class which uses the header date and time to set the clock
* **NTPClient**: a NTP based time class written by Fabrice Weinberg
* **WundergroundClient**: fetches current weather and forecast from wunderground.com
* **ThingspeakClient**: fetches data from Thingspeak which you might have collected with another sensor node and posted there. I use this to measure outdoor temperature and humidity and show it on the WeatherStation with a ClimateNode: https://shop.squix.ch/index.php/esp8266/climatenode.html  

## Why Weather Station as a library?

I realized that more and more the Weather Station was becoming a general framework for displaying data over WiFi to one of these pretty displays. But everyone would have different ways or sources for data and having the important part of the library would rather be the classes which fetch the data then the main class.
So if you write data fetchers which might be of interest to others please contact me to integrate them here or offer your code as extension library yourself and call it something like esp8266-weather-station-<yourservice>.
I will gladly list it here as third party library...
