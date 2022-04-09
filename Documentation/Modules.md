## Available Modules
* **Time Client**: simple class which uses the header date and time to set the clock
* **NTP Client**: a NTP based time class written by Fabrice Weinberg
* **OpenWeatherMap Client**: A REST client for the OpenWeatherMap.com service, providing weather information
* **Aeris Client**: Client for the service provided by aerisweather.com. Fully functional initial version. After the Wunderground incident (see [upgrade notes](#upgrade-notes)) we first targeted Aeris before we settled with OpenWeatherMap. This code is unmaintained but will remain part of this library for the time being.
* **Thingspeak Client**: fetches data from Thingspeak which you might have collected with another sensor node and posted there.
* **Astronomy**: algorithms to calculate current lunar phase and illumination.
* **SunMoonCalc**: a calculator for sun and moon properties for a given date & time and location. This implementation is port of a [Java class by T. Alonso Albi][Alonso] from OAN (Spain).