# WeatherStationDemoExtendedDST

Daylight Saving Time and other customizations of the original ESP8266 OLED Weather Station.
Uses SSD1306 128x64 OLED display with with either SPI or I2C interface

| New Splash Screen | DHT22 Update |
|:-----------------:|:------------:|
| ![cover](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/splash.jpg) | ![DHT22 Update](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/DHT22_update.jpg) |

| Zurich Standard Time | Boston Daylight Saving Time | Conditions Screen |
|:--------------------:|:---------------------------:|:-----------------:|
| ![Zurich CET](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/Zurich_CET.jpg) | ![Boston EDT](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/Boston_EDT.jpg) | ![Conditions](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/conditions.jpg) |

| DHT Sensor | Thingspeak Sensor | 1 to 3 Day Forecast | 4 to 6 Day Forecast |
|:----------:|:-----------------:|:-------------------:|:-------------------:|
| ![DHT Sensor](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/DHT22_sensor.jpg) | ![ThingSpeak Sensor](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/thingspeak.jpg) |  ![DHT Sensor](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/1-3_day_forecast.jpg) | ![ThingSpeak Sensor](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/4-6_day_forecast.jpg) |

## Specific customizations include:

*  Added Wifi Splash screen and credit to Squix78
*  Modified progress bar to a thicker and symmetrical shape
*  Replaced TimeClient with built-in lwip sntp client (no need for external ntp client library)
*  Added Daylight Saving Time Auto adjuster with DST rules using simpleDSTadjust library
 * https://github.com/neptune2/simpleDSTadjust
*  Added Locale Setting examples for Boston, Zurich and Sydney
 * Selectable NTP servers for each locale
 * DST rules and timezone settings customizable for each locale
  * See https://www.timeanddate.com/time/change/ for DST rules
 * Added AM/PM or 24-hour option for each locale
 * Added metric settings for each locale
*  Changed Clock screen to 7-segment font from http://www.keshikan.net/fonts-e.html
*  Added Forecast screen for days 4-6 (requires 1.1.3 or later version of esp8266_Weather_Station library)
*  Added support for DHT22, DHT21 and DHT11 Indoor Temperature and Humidity Sensors
*  Fixed bug preventing display.flipScreenVertically() from working
*  Slight adjustments to overlay
*  Moved user settings into settings.h
*  Added #defines in settings.h to make it easier to select SPI or I2C interface for OLED in settings.h


## Hardware Requirements

This code is made for an 128x64 SSD1603 OLED display with code running on an ESP8266.
Either the SPI or I2C version can be used.
You can buy the original Squix78 Weather Station Kit here: 
[Squix Shop](https://shop.squix.ch/index.php/esp8266.html) or here: [US Amazon store](https://www.amazon.com/gp/product/B01KE7BA3O)

## Software Requirements/ Libraries

* [Arduino IDE with ESP8266 platform installed](https://github.com/esp8266/Arduino)
* [Weather Station Library](https://github.com/squix78/esp8266-weather-station) or through Library Manager
* [ESP8266 OLED SSD1306 Library](https://github.com/squix78/esp8266-oled-ssd1306)
* [WifiManager Library](https://github.com/tzapu/WiFiManager)

### Additional required library for automatic Daylight Saving Time adjust
* [simpleDSTadjust Library](https://github.com/neptune2/simpleDSTadjust)

You also need to get an API key for the Wunderground data: https://www.wunderground.com/

## Wemos D1R2 Wiring
![Wemos D1R2 and DHT22 sensor](https://github.com/neptune2/esp8266-weather-station-oled-DST/raw/master/resources/cover.jpg)

See code for pin configurations

| SSD1306 SPI | Wemos D1R2 |
| ----------- |:----------:|
| CS          | D8         |
| DC          | D2         |
| RST         | D0         |
| D1          | D7         |
| D0          | D5         |
| GND         | GND        |
| VCC         | 3V3        |

| DHT22 | Wemos D1R2 |
| ----- |:----------:| 
| DATA  | D4         |
| GND   | GND        |
| VCC   | 3V3        |
