## Upgrade Notes

**Version 2, January 2020, removes WU support, see below**

**Replace Wunderground with OpenWeatherMap as weather data provider**

The weather information provider we used so far (Wunderground) [recently stopped their free tier][No Free] without previous notice on May 15, 2018. This release adds support for a new provider with a free tier for weather information: OpenWeatherMap.com. The basic demo (WeatherStationDemo) has been adapted to use this new API through the OpenWeatherMapCurrent and OpenWeatherMapForecast REST clients.

Sadly OpenWeatherMap provides less information than Wunderground did (or still does). If you are missing attributes in the response documents then please [contact the OpenWeatherMap team][OpenWeatherMap].

**ESP8266 OLED Library upgrade**

The ESP8266 OLED Library changed a lot with the latest release of version 3.0.0. We fixed many bugs and improved performance and changed the API a little bit. This means that you might have to adapt your Weather Station Code if you created it using the older 2.x.x version of the library. Either compare your code to the updated WeatherStationDemo or read through the [upgrade guide][Upgrade]