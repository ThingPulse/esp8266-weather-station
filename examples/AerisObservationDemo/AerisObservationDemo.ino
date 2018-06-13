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

#include <ESP8266WiFi.h>
#include <JsonListener.h>
#include "AerisObservations.h"

/**
 * Aeris Weather
 */

// initiate the WundergoundClient
AerisObservations observations;

String AERIS_CLIENT_ID = "tWOmsRUXe4EFTHQKmUKOK";
String AERIS_SECRET_KEY = "gRoMoapOyg46HwB7dRmoVPaJ0vUgAiud1CFWuLfF";
String AERIS_LOCATION = "Zurich,CH";

/**
 * WiFi Settings
 */
const char* ESP_HOST_NAME = "esp-" + ESP.getFlashChipId();
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


/**
 * SETUP
 */
void setup() {
  Serial.begin(115200);
  delay(500);
  connectWifi();

  Serial.println();
  Serial.println("\n\nNext Loop-Step: " + String(millis()) + ":");

  AerisObservationsData observationData;
  observations.updateObservations(&observationData, AERIS_CLIENT_ID, AERIS_SECRET_KEY, AERIS_LOCATION);

  Serial.println("------------------------------------");


  // uint64_t timestamp;
  Serial.printf("timestamp: %d\n", observationData.timestamp);
  // String dateTimeISO;
  Serial.printf("dateTimeISO: %s\n", observationData.dateTimeISO.c_str());
  // sint16_t tempC;
  Serial.printf("tempC: %d\n", observationData.tempC);
  // sint16_t tempF;
  Serial.printf("tempF: %d\n", observationData.tempF);
  // sint16_t dewpointC;
  Serial.printf("dewpointC: %d\n", observationData.dewpointC);
  // sint16_t dewpointF;
  Serial.printf("dewpointF: %d\n", observationData.dewpointF);
  // uint8_t humidity;
  Serial.printf("humidity: %d\n", observationData.humidity);
  // uint16_t pressureMB;
  Serial.printf("pressureMB: %d\n", observationData.pressureMB);
  // float pressureIN;
  Serial.printf("pressureIN: %f\n", observationData.pressureIN);
  // uint16_t spressureMB;
  Serial.printf("spressureMB: %d\n", observationData.spressureMB);
  // float spressureIN;
  Serial.printf("spressureIN: %f\n", observationData.spressureIN);
  // uint16_t altimeterMB;
  Serial.printf("altimeterMB: %d\n", observationData.altimeterMB);
  // float altimeterIN;
  Serial.printf("altimeterIN: %f\n", observationData.altimeterIN);
  // uint16_t windSpeedKTS;
  Serial.printf("windSpeedKTS: %d\n", observationData.windSpeedKTS);
  // uint16_t windSpeedKPH;
  Serial.printf("windSpeedKPH: %d\n", observationData.windSpeedKPH);
  // uint16_t windSpeedMPH;
  Serial.printf("windSpeedMPH: %d\n", observationData.windSpeedMPH);
  // uint16_t windDirDEG;
  Serial.printf("windDirDEG: %d\n", observationData.windDirDEG);
  // String windDir;
  Serial.printf("windDir: %s\n", observationData.windDir.c_str());
  // uint16_t windGustKTS;
  Serial.printf("windGustKTS: %d\n", observationData.windGustKTS);
  // uint16_t windGustKPH;
  Serial.printf("windGustKPH: %d\n", observationData.windGustKPH);
  // uint16_t windGustMPH;
  Serial.printf("windGustMPH: %d\n", observationData.windGustMPH);
  // String flightRule;
  Serial.printf("flightRule: %s\n", observationData.flightRule.c_str());
  // float visibilityKM;
  Serial.printf("visibilityKM: %f\n", observationData.visibilityKM);
  // float visibilityMI;
  Serial.printf("visibilityMI: %f\n", observationData.visibilityMI);
  // String weather;
  Serial.printf("weather: %s\n", observationData.weather.c_str());
  // String weatherShort;
  Serial.printf("weatherShort: %s\n", observationData.weatherShort.c_str());
  // String weatherCoded;
  Serial.printf("weatherCoded: %s\n", observationData.weatherCoded.c_str());
  // String weatherPrimary;
  Serial.printf("weatherPrimary: %s\n", observationData.weatherPrimary.c_str());
  // String weatherPrimaryCoded;
  Serial.printf("weatherPrimaryCoded: %s\n", observationData.weatherPrimaryCoded.c_str());
  // String cloudsCoded;
  Serial.printf("cloudsCoded: %s\n", observationData.cloudsCoded.c_str());
  // String icon;
  Serial.printf("icon: %s\n", observationData.icon.c_str());
  // String iconMeteoCon;
  Serial.printf("iconMeteoCon: %s\n", observationData.iconMeteoCon.c_str());
  // sint16_t heatindexC;
  Serial.printf("heatindexC: %d\n", observationData.heatindexC);
  // sint16_t heatindexF;
  Serial.printf("heatindexF: %d\n", observationData.heatindexF);
  // sint16_t windchillC;
  Serial.printf("windchillC: %d\n", observationData.windchillC);
  // sint16_t windchillF;
  Serial.printf("windchillF: %d\n", observationData.windchillF);
  // sint16_t feelslikeC;
  Serial.printf("feelslikeC: %d\n", observationData.feelslikeC);
  // sint16_t feelslikeF;
  Serial.printf("feelslikeF: %d\n", observationData.feelslikeF);
  // boolean isDay;
  Serial.printf("isDay: %d\n", observationData.isDay);
  // uint64_t sunrise;
  Serial.printf("sunrise: %d\n", observationData.sunrise);
  // String sunriseISO;
  Serial.printf("sunriseISO: %s\n", observationData.sunriseISO.c_str());
  // uint64_t sunset;
  Serial.printf("sunset: %d\n", observationData.sunset);
  // String sunsetISO;
  Serial.printf("sunsetISO: %s\n", observationData.sunsetISO.c_str());
  // uint16_t snowDepthCM;
  Serial.printf("snowDepthCM: %d\n", observationData.snowDepthCM);
  // uint16_t snowDepthIN;
  Serial.printf("snowDepthIN: %d\n", observationData.snowDepthIN);
  // uint16_t precipMM;
  Serial.printf("precipMM: %d\n", observationData.precipMM);
  // uint16_t precipIN;
  Serial.printf("precipIN: %d\n", observationData.precipIN);
  // uint16_t solradWM2;
  Serial.printf("solradWM2: %d\n", observationData.solradWM2);
  // String solradMethod;
  Serial.printf("solradMethod: %s\n", observationData.solradMethod.c_str());
  // uint16_t light;
  Serial.printf("light: %d\n", observationData.light);
  // uint16_t sky;
  Serial.printf("sky: %d\n", observationData.sky);
  Serial.println();
  Serial.println("---------------------------------------------------/\n");

}


/**
 * LOOP
 */
void loop() {

}
