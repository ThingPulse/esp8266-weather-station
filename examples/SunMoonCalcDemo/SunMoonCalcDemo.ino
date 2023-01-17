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
#include <ESPHTTPClient.h>
#include <time.h>
#include "SunMoonCalc.h"

/**
 * WiFi Settings
 */
const char* WIFI_SSID     = "yourssid";
const char* WIFI_PASSWORD = "yourpassw0rd";

// initiate the WifiClient
WiFiClient wifiClient;

/**
 * Helper funtions
 */
void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());
}

void printResult(SunMoonCalc::Result result) {
  Serial.println("Sun");
  Serial.println("\tRise: " + formatTime(result.sun.rise));
  Serial.println("\tNoon: " + formatTime(result.sun.transit));
  Serial.println("\tSet: " + formatTime(result.sun.set));
  Serial.printf("\tAzimuth: %f°\n", result.sun.azimuth);
  Serial.printf("\tElevation: %f°\n", result.sun.elevation);
  Serial.printf("\tDistance: %fkm\n", result.sun.distance);
  Serial.println("Moon");
  Serial.println("\tRise: " + formatTime(result.moon.rise));
  Serial.println("\tNoon: " + formatTime(result.moon.transit));
  Serial.println("\tSet: " + formatTime(result.moon.set));
  Serial.printf("\tAzimuth: %f°\n", result.moon.azimuth);
  Serial.printf("\tElevation: %f°\n", result.moon.elevation);
  Serial.printf("\tDistance: %fkm", result.moon.distance);
  Serial.printf("\tAge: %f days\n", result.moon.age);
  Serial.printf("\tIllumination: %f%\n", result.moon.illumination * 100);
  Serial.println("\tPhase: " + result.moon.phase.name);
  Serial.printf("\tBright limb angle: %frad\n", result.moon.brightLimbAngle);
  Serial.printf("\tPosition angle of axis: %frad\n", result.moon.axisPositionAngle);
  Serial.printf("\tParallactic angle: %frad\n", result.moon.parallacticAngle);
}

String padWithZeroBelowTen(int d) {
  return d < 10 ? "0" + String(d) : String(d);
}

String formatTime(time_t timestamp) {
  tm *date = gmtime(&timestamp);
  String year = "" + String(date->tm_year + 1900);
  String month = padWithZeroBelowTen(date->tm_mon + 1);
  String day = padWithZeroBelowTen(date->tm_mday);
  return year + "-" + month + "-" + day + " " + padWithZeroBelowTen(date->tm_hour) + ":" +
         padWithZeroBelowTen(date->tm_min) + ":" + padWithZeroBelowTen(date->tm_sec) + " UTC";
}
// END helper functions

void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(500);
  connectWifi();
  Serial.println();
  
  Serial.println("Syncing time...");
  configTime(0, 0, "pool.ntp.org");  
  // some explanations about this POSIX format: https://www.ibm.com/developerworks/aix/library/au-aix-posix/
  // -> represents a central European timezone identifier such as Europe/Berlin
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 0);
  while(time(nullptr) <= 100000) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Time sync'ed");

  // prepare the time input value
  time_t tnow = time(nullptr);
  Serial.println(String(ctime(&tnow)));
  
  // 'now' has to be UTC, lat/lng in degrees not raadians
  SunMoonCalc smCalc = SunMoonCalc(tnow, 47.366, 8.533);
  const SunMoonCalc::Result result = smCalc.calculateSunAndMoonData();

  // for reference you may want to compare results (remember: they're in UTC!) to https://www.timeanddate.com/moon/
  printResult(result);
}

void loop() {
  // no-op, only run the code once
}
