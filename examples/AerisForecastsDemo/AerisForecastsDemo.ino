/**The MIT License (MIT)

Copyright (c) 2018 by ThingPulse Ltd., https://thingpulse.com

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
#include "AerisForecasts.h"

/**
 * Aeris Weather
 */
#define MAX_FORECASTS 3
AerisForecasts client;

String AERIS_CLIENT_ID = "tWOmsRUXe4EFTHQKmUKOK";
String AERIS_SECRET_KEY = "gRoMoapOyg46HwB7dRmoVPaJ0vUgAiud1CFWuLfF";
String AERIS_LOCATION = "Zurich,CH";

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

void print(const char* name, String value) {
  Serial.printf("%s: %s\n", name, value.c_str());
}

void print(String name, uint64_t value) {
  Serial.printf("%s: %d\n", name.c_str(), value);
}

void print(String name, uint32_t value) {
  Serial.printf("%s: %d\n", name.c_str(), value);
}

void print(String name, uint16_t value) {
  Serial.printf("%s: %d\n", name.c_str(), value);
}

void print(String name, int16_t value) {
  Serial.printf("%s: %d\n", name.c_str(), value);
}

void print(String name, uint8_t value) {
  Serial.printf("%s: %d\n", name.c_str(), value);
}

void print(String name, float value) {
  Serial.printf("%s: %f\n", name.c_str(), value);
}

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


/**
 * SETUP
 */
void setup() {
  Serial.begin(115200);
  delay(500);
  connectWifi();

  Serial.println();
  Serial.println("\n\nNext Loop-Step: " + String(millis()) + ":");

  AerisForecastData forecasts[3];
  client.updateForecasts(forecasts, AERIS_CLIENT_ID, AERIS_SECRET_KEY, AERIS_LOCATION, MAX_FORECASTS);

  Serial.println("------------------------------------");
  for (int i = 0; i < MAX_FORECASTS; i++) {
    // uint64_t timestamp; //  1526706000
    print("timestamp", forecasts[i].timestamp);
    // String validTime; // "2018-05-19T07:00:00+02:00"
    print("validTime", forecasts[i].validTime);
    // String dateTimeISO; //"2018-05-19T07:00:00+02:00"
    print("dateTimeISO", forecasts[i].dateTimeISO);
    // sint16_t maxTempC; //20
    print("maxTempC", forecasts[i].maxTempC);
    // sint16_t maxTempF; //69
    print("maxTempF", forecasts[i].maxTempF);
    // sint16_t minTempC; //14
    print("minTempC", forecasts[i].minTempC);
    // sint16_t minTempF; // 56
    print("minTempF", forecasts[i].minTempF);
    // sint16_t avgTempC; // 17
    print("avgTempC", forecasts[i].avgTempC);
    // sint16_t avgTempF; // 62
    print("avgTempF", forecasts[i].avgTempF);
    // sint16_t tempC; // null
    print("tempC", forecasts[i].tempC);
    // sint16_t tempF; // null
    print("tempF", forecasts[i].tempF);
    // sint16_t pop; // 20
    print("pop", forecasts[i].pop);
    // float precipMM; // 3.53
    print("precipMM", forecasts[i].precipMM);
    // float precipIN; // 0.14
    print("precipIN", forecasts[i].precipIN);
    // float iceaccum; // null
    print("iceaccum", forecasts[i].iceaccum);
    // float iceaccumMM; // null
    print("iceaccumMM", forecasts[i].iceaccumMM);
    // float iceaccumIN; // null
    print("iceaccumIN", forecasts[i].iceaccumIN);
    // uint8_t maxHumidity; // 82
    print("maxHumidity", forecasts[i].maxHumidity);
    // uint8_t minHumidity; // 53
    print("minHumidity", forecasts[i].minHumidity);
    // uint8_t humidity; // 68
    print("humidity", forecasts[i].humidity);
    // uint8_t uvi; // 6
    print("uvi", forecasts[i].uvi);
    // uint16_t pressureMB; // 1018
    print("pressureMB", forecasts[i].pressureMB);
    // float pressureIN; // 30.06
    print("pressureIN", forecasts[i].pressureIN);
    // uint8_t sky; // 99
    print("sky", forecasts[i].sky);
    // uint16_t snowCM; // 0
    print("snowCM", forecasts[i].snowCM);
    // uint16_t snowIN; // 0
    print("snowIN", forecasts[i].snowIN);
    // sint16_t feelslikeC; // 14
    print("feelslikeC", forecasts[i].feelslikeC);
    // sint16_t feelslikeF; // 56
    print("feelslikeF", forecasts[i].feelslikeF);
    // sint16_t minFeelslikeC; // 14
    print("minFeelslikeC", forecasts[i].minFeelslikeC);
    // sint16_t minFeelslikeF; // 56
    print("minFeelslikeF", forecasts[i].minFeelslikeF);
    // sint16_t maxFeelslikeC; // 20
    print("maxFeelslikeC", forecasts[i].maxFeelslikeC);
    // sint16_t maxFeelslikeF; // 69
    print("maxFeelslikeF", forecasts[i].maxFeelslikeF);
    // sint16_t avgFeelslikeC; // 17
    print("avgFeelslikeC", forecasts[i].avgFeelslikeC);
    // sint16_t avgFeelslikeF; // 63
    print("avgFeelslikeF", forecasts[i].avgFeelslikeF);
    // sint16_t dewpointC; // 11
    print("dewpointC", forecasts[i].dewpointC);
    // sint16_t dewpointF; // 51
    print("dewpointF", forecasts[i].dewpointF);
    // sint16_t maxDewpointC; // 13
    print("maxDewpointC", forecasts[i].maxDewpointC);
    // sint16_t maxDewpointF; // 55
    print("maxDewpointF", forecasts[i].maxDewpointF);
    // sint16_t minDewpointC; // 10
    print("minDewpointC", forecasts[i].minDewpointC);
    // sint16_t minDewpointF; // 51
    print("minDewpointF", forecasts[i].minDewpointF);
    // sint16_t avgDewpointC; // 11
    print("avgDewpointC", forecasts[i].avgDewpointC);
    // sint16_t avgDewpointF; // 52
    print("avgDewpointF", forecasts[i].avgDewpointF);
    // uint16_t windDirDEG; // 2
    print("windDirDEG", forecasts[i].windDirDEG);
    // String windDir; // "N"
    print("windDir", forecasts[i].windDir);
    // uint16_t windDirMaxDEG; // 40
    print("windDirMaxDEG", forecasts[i].windDirMaxDEG);
    // String windDirMax; // "NE"
    print("windDirMax", forecasts[i].windDirMax);
    // sint16_t windDirMinDEG; // 39
    print("windDirMinDEG", forecasts[i].windDirMinDEG);
    // String windDirMin; // "NE"
    print("windDirMin", forecasts[i].windDirMin);
    // uint16_t windGustKTS; // 6
    print("windGustKTS", forecasts[i].windGustKTS);
    // uint16_t windGustKPH; // 11
    print("windGustKPH", forecasts[i].windGustKPH);
    // uint16_t windGustMPH; // 7
    print("windGustMPH", forecasts[i].windGustMPH);
    // uint16_t windSpeedKTS; // 4
    print("windSpeedKTS", forecasts[i].windSpeedKTS);
    // uint16_t windSpeedKPH; // 7
    print("windSpeedKPH", forecasts[i].windSpeedKPH);
    // uint16_t windSpeedMPH; // 5
    print("windSpeedMPH", forecasts[i].windSpeedMPH);
    // uint16_t windSpeedMaxKTS; // 6
    print("windSpeedMaxKTS", forecasts[i].windSpeedMaxKTS);
    // uint16_t windSpeedMaxKPH; // 11
    print("windSpeedMaxKPH", forecasts[i].windSpeedMaxKPH);
    // uint16_t windSpeedMaxMPH; // 7
    print("windSpeedMaxMPH", forecasts[i].windSpeedMaxMPH);
    // uint16_t windSpeedMinKTS; // 1
    print("windSpeedMinKTS", forecasts[i].windSpeedMinKTS);
    // uint16_t windSpeedMinKPH; // 2
    print("validTime", forecasts[i].windSpeedMinKPH);
    // uint16_t windSpeedMinMPH; // 1
    print("windSpeedMinMPH", forecasts[i].windSpeedMinMPH);
    // uint16_t windDir80mDEG; // 5
    print("windDir80mDEG", forecasts[i].windDir80mDEG);
    // String windDir80m; // "N"
    print("windDir80m", forecasts[i].windDir80m);
    // uint16_t windDirMax80mDEG; // 40
    print("windDirMax80mDEG", forecasts[i].windDirMax80mDEG);
    // String windDirMax80m; // "NE"
    print("windDirMax80m", forecasts[i].windDirMax80m);
    // uint16_t windDirMin80mDEG; // 39
    print("windDirMin80mDEG", forecasts[i].windDirMin80mDEG);
    // String windDirMin80m; // "NE"
    print("windDirMin80m", forecasts[i].windDirMin80m);
    // uint16_t windGust80mKTS; // 9
    print("windGust80mKTS", forecasts[i].windGust80mKTS);
    // uint16_t windGust80mKPH; // 17
    print("windGust80mKPH", forecasts[i].windGust80mKPH);
    // uint16_t windGust80mMPH; // 11
    print("windGust80mMPH", forecasts[i].windGust80mMPH);
    // uint16_t windSpeed80mKTS; // 6
    print("windSpeed80mKTS", forecasts[i].windSpeed80mKTS);
    // uint16_t windSpeed80mKPH; // 11
    print("windSpeed80mKPH", forecasts[i].windSpeed80mKPH);
    // uint16_t windSpeed80mMPH; // 7
    print("windSpeed80mMPH", forecasts[i].windSpeed80mMPH);
    // uint16_t windSpeedMax80mKTS; // 9
    print("windSpeedMax80mKTS", forecasts[i].windSpeedMax80mKTS);
    // uint16_t windSpeedMax80mKPH; // 17
    print("windSpeedMax80mKPH", forecasts[i].windSpeedMax80mKPH);
    // uint16_t windSpeedMax80mMPH; // 11
    print("windSpeedMax80mMPH", forecasts[i].windSpeedMax80mMPH);
    // uint16_t windSpeedMin80mKTS; // 4
    print("windSpeedMin80mKTS", forecasts[i].windSpeedMin80mKTS);
    // uint16_t windSpeedMin80mKPH; // 7
    print("windSpeedMin80mKPH", forecasts[i].windSpeedMin80mKPH);
    // uint16_t windSpeedMin80mMPH; // 4
    print("windSpeedMin80mMPH", forecasts[i].windSpeedMin80mMPH);
    // String weather; // "Cloudy with Drizzle"
    print("weather", forecasts[i].weather);
    // String weatherPrimary; // "Drizzle"
    print("weatherPrimary", forecasts[i].weatherPrimary);
    // String weatherPrimaryCoded; // "IS:VL:RW"
    print("weatherPrimaryCoded", forecasts[i].weatherPrimaryCoded);
    // String cloudsCoded; // "OV"
    print("cloudsCoded", forecasts[i].cloudsCoded);
    // String icon; // "drizzle.png"
    print("icon", forecasts[i].icon);
    // String iconMeteoCon; // Q
    print("iconMeteoCon", forecasts[i].iconMeteoCon);
    // boolean isDay; // true
    print("isDay", forecasts[i].isDay ? "true" : "false");
    // uint64_t sunrise; // 1526701425
    print("sunrise", forecasts[i].sunrise);
    // String sunriseISO; // "2018-05-19T05:43:45+02:00"
    print("sunriseISO", forecasts[i].sunriseISO);
    // uint64_t sunset; // 1526756450
    print("sunset", forecasts[i].sunset);
    // String sunsetISO; // "2018-05-19T21:00:50+02:00"
    print("sunsetISO", forecasts[i].sunsetISO);

    Serial.println();
    Serial.println("---------------------------------------------------/\n");
  }
}


/**
 * LOOP
 */
void loop() {

}
