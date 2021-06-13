/**The MIT License (MIT)

Copyright (c) 2020 by Chris Klinger, https://chrisklinger.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <JsonListener.h>
#include <time.h>
#include "OpenWeatherMapOneCall.h"

// See https://docs.thingpulse.com/how-tos/openweathermap-key/
String OPEN_WEATHER_MAP_APP_ID = "changeme";
/*
Go to https://www.latlong.net/ and search for a location. Go through the
result set and select the entry closest to the actual location you want to display
data for. Use Latitude and Longitude values here.
 */
 float OPEN_WEATHER_MAP_LOCATTION_LAT = 52.520008; // Berlin, DE
 float OPEN_WEATHER_MAP_LOCATTION_LON = 13.404954; // Berlin, DE
/*
Arabic - ar, Bulgarian - bg, Catalan - ca, Czech - cz, German - de, Greek - el,
English - en, Persian (Farsi) - fa, Finnish - fi, French - fr, Galician - gl,
Croatian - hr, Hungarian - hu, Italian - it, Japanese - ja, Korean - kr,
Latvian - la, Lithuanian - lt, Macedonian - mk, Dutch - nl, Polish - pl,
Portuguese - pt, Romanian - ro, Russian - ru, Swedish - se, Slovak - sk,
Slovenian - sl, Spanish - es, Turkish - tr, Ukrainian - ua, Vietnamese - vi,
Chinese Simplified - zh_cn, Chinese Traditional - zh_tw.
*/
String OPEN_WEATHER_MAP_LANGUAGE = "en";
boolean IS_METRIC = false;

/**
 * WiFi Settings
 */
#if defined(ESP8266)
const char* ESP_HOST_NAME = "esp-" + ESP.getFlashChipId();
#else
const char* ESP_HOST_NAME = "esp-" + ESP.getEfuseMac();
#endif
const char* WIFI_SSID     = "yourssid";
const char* WIFI_PASSWORD = "yourpassw0rd";

// initiate the WifiClient
WiFiClient wifiClient;


/**
 * Helping funtions
 */
void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());
  Serial.println();
}

OpenWeatherMapOneCallData openWeatherMapOneCallData;

/**
 * SETUP
 */
void setup() {

  Serial.begin(115200);
  delay(500);
  connectWifi();

  Serial.println();

  OpenWeatherMapOneCall *oneCallClient = new OpenWeatherMapOneCall();
  oneCallClient->setMetric(IS_METRIC);
  oneCallClient->setLanguage(OPEN_WEATHER_MAP_LANGUAGE);

  long executionStart = millis();
  oneCallClient->update(&openWeatherMapOneCallData, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATTION_LAT, OPEN_WEATHER_MAP_LOCATTION_LON);
  delete oneCallClient;
  oneCallClient = nullptr;

  Serial.println( "Current Weather: ");
  Serial.println( String(openWeatherMapOneCallData.current.temp, 1) + (IS_METRIC ? "°C" : "°F") );
  Serial.println( openWeatherMapOneCallData.current.weatherDescription );

  Serial.println( "Forecasts: ");

  for(int i = 0; i < 5; i++)
  {
    if(openWeatherMapOneCallData.daily[i].dt > 0) {
      Serial.println("dt: " + String(openWeatherMapOneCallData.daily[i].dt) );
      Serial.println("temp: " + String(openWeatherMapOneCallData.daily[i].tempDay, 1) + (IS_METRIC ? "°C" : "°F") );
      Serial.println("desc: " +  openWeatherMapOneCallData.daily[i].weatherDescription);
      Serial.println();
    }
  }

}


/**
 * LOOP
 */
void loop() {

}
