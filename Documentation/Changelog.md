
# Changelog

## Version 2

**When:** `January 2020` <br>
**What:** `Removes Wunderground Support`

<br>

### Wunderground ➞ OpenWeatherMap

*Weather Data Provider Replacement*


**Wunderground**, the weather information provider <br>
we used so far, [stopped their free tier][No Free] without notice <br>
on `May 15, 2018` .

This release adds support for `OpenWeatherMap.com` , an <br>
alternate provider with a free tier for weather information.

The `WeatherStationDemo` has been adapted to use <br>
this new **API** through the `OpenWeatherMapCurrent` <br>
and `OpenWeatherMapForecast` **REST** clients.

<br>

*Sadly **OpenWeatherMap** provides less* <br>
*information than **Wunderground** does.*

<br>

If you are missing attributes in the response docs <br>
then please contact the **[OpenWeatherMap]** team.

<br>

### OLED Library Upgrade

The **ESP8266 OLED Library** changed <br>
a lot with the latest release of `v3.0.0` .

We fixed many bugs, changed the **API** <br>
a little bit and improved performance.

This means that you might have to adapt <br>
your ***Weather Station Code*** if you created <br>
it using older `2.x.x` versions.

Either compare your code to the updated <br>
**WeatherStationDemo** or read the guide <br>
on how to **[Upgrade]**.


<!----------------------------------------------------------------------------->

[Upgrade]: https://github.com/ThingPulse/esp8266-oled-ssd1306/blob/master/UPGRADE-3.0.md
[OpenWeatherMap]:    https://openweathermap.desk.com/customer/portal/emails/new
[No Free]:           https://thingpulse.com/weather-underground-no-longer-providing-free-api-keys/