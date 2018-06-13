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
#include "AerisSunMoon.h"

/**
 * Aeris Weather
 */

// initiate the Aeris client
AerisSunMoon sunMoonClient;

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

  AerisSunMoonData sunMoonData;
  sunMoonClient.updateSunMoon(&sunMoonData, AERIS_CLIENT_ID, AERIS_SECRET_KEY, AERIS_LOCATION);

  Serial.println("------------------------------------");

  // uint64_t sunRise; // "rise":1493291184,
  Serial.printf("sunRise: %d\n", sunMoonData.sunRise);
  // String sunRiseISO; // "riseISO":"2017-04-27T06:06:24-05:00",
  Serial.printf("sunRiseISO: %s\n", sunMoonData.sunRiseISO.c_str());
  // uint64_t sunSet; // "set":1493342079,
  Serial.printf("sunSet: %d\n", sunMoonData.sunSet);
  // String sunSetISO; //"setISO":"2017-04-27T20:14:39-05:00",
  Serial.printf("sunSetISO: %s\n", sunMoonData.sunSetISO.c_str());
  // uint64_t sunTransit; // "transit":1493316631,
  Serial.printf("sunTransit: %d\n", sunMoonData.sunTransit);
  // String sunTransitISO; // "transitISO":"2017-04-27T13:10:31-05:00",
  Serial.printf("sunTransitISO: %s\n", sunMoonData.sunTransitISO.c_str());
  // boolean midnightSun; // "midnightSun":false,
  Serial.printf("midnightSun: %d\n", sunMoonData.midnightSun);
  // boolean polarNight; // "polarNight":false,
  Serial.printf("polarNight: %d\n", sunMoonData.polarNight);
  // uint64_t moonRise; //"rise":1493295480,
  Serial.printf("moonRise: %d\n", sunMoonData.moonRise);
  // String moonRiseISO; // "riseISO":"2017-04-27T07:18:00-05:00",
  Serial.printf("moonRiseISO: %s\n", sunMoonData.moonRiseISO.c_str());
  // uint64_t moonSet; // "set":1493347800,
  Serial.printf("moonSet: %d\n", sunMoonData.moonSet);
  // String moonSetISO; // "setISO":"2017-04-27T21:50:00-05:00",
  Serial.printf("moonSetISO: %s\n", sunMoonData.moonSetISO.c_str());
  // uint64_t moonTransit; // "transit":1493321340,
  Serial.printf("moonTransit: %d\n", sunMoonData.moonTransit);
  // String moonTransitISO; // "transitISO":"2017-04-27T14:29:00-05:00",
  Serial.printf("moonTransitISO: %s\n", sunMoonData.moonTransitISO.c_str());
  // uint64_t moonUnderfoot; // "underfoot":1493276400,
  Serial.printf("moonUnderfoot: %d\n", sunMoonData.moonUnderfoot);
  // String moonUnderfootISO; // "underfootISO":"2017-04-27T02:00:00-05:00",
  Serial.printf("moonUnderfootISO: %s\n", sunMoonData.moonUnderfootISO.c_str());
  // float moonPhase; // "phase":0.0516,
  Serial.printf("moonPhase: %f\n", sunMoonData.moonPhase);
  // String moonPhaseName; // "name":"waxing crescent",
  Serial.printf("moonPhaseName: %s\n", sunMoonData.moonPhaseName.c_str());
  // uint8_t moonIllum; // "illum":3,
  Serial.printf("moonIllum: %d\n", sunMoonData.moonIllum);
  // float moonAge; // "age":1.52,
  Serial.printf("moonAge: %f\n", sunMoonData.moonAge);
  // float moonAngle; // "angle":0.55
  Serial.printf("moonAngle: %f\n", sunMoonData.moonAngle);
  Serial.println();
  Serial.println("---------------------------------------------------/\n");

}


/**
 * LOOP
 */
void loop() {

}
