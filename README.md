# esp8266-weather-station

New version of the ESP8266 Weather Station

## Setup

* Download this project either with a git checkout or press "Download as zip"
* Install the following librarys with your Arduino Library Manager in Sketch > Include Library > Manage Libraries...
 * Json Streaming Parser (by Daniel Eichhorn)
 * ESP8266 Oled Driver for SSD1306 display (by me as well)
* Go to http://wunderground.com, create an account and get an API Key
* Open the sketch in the Arduino Include and
 * Enter  the Wunderground API Key
 * Enter your Wifi credentials
 * Adjust the location according to Wunderground API, e.g. Zurich, CH
 * Adjust UTC offset

## Known issues
Many people asked me to finally publish the new version. I'm doing that now, knowing that some things are not perfect or stable enough. If you detect or even fix one of these issues, please contact me or create a pull request
* Time is running out of sync. Sometimes off my many minutes. I assume it has something todo with the implementation of the millis() function which I based the time sync upon
* The precipitation value from Wunderground currently always returns 0.0mm for my location. Maybe this looks better in other locations
* Sometimes the WeatherStation crashes. I didn't have time to debug this behavior. If you have any clue, please let me know.
