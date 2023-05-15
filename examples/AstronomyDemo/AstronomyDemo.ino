/**The MIT License (MIT)

Copyright (c) 2018 by ThingPulse

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

See more at https://thingpulse.com
*/

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <ESPHTTPClient.h>
#include <time.h>
#include "Astronomy.h"

/**
 * WiFi Settings
 */
const char* WIFI_SSID     = "yourssid";
const char* WIFI_PASSWORD = "yourpassw0rd";

// initiate the WifiClient
WiFiClient wifiClient;

#define UTC_OFFSET 1
#define DST_OFFSET 1
#define NTP_SERVERS "us.pool.ntp.org", "time.nist.gov", "pool.ntp.org"
#define EPOCH_1970

const String MOON_PHASES[] = {"New Moon", "Waxing Crescent", "First Quarter", "Waxing Gibbous",
                              "Full Moon", "Waning Gibbous", "Third quarter", "Waning Crescent"};

Astronomy astronomy;

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

void setup() {
  Serial.begin(115200);
  delay(500);
  connectWifi();
  Serial.println();
  configTime(UTC_OFFSET, DST_OFFSET * 60, NTP_SERVERS);
  Serial.println("Syncing time...");
  while(time(nullptr) <= 10000) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Time sync'ed");

  // prepare the input values
  time_t now = time(nullptr);
  struct tm * timeinfo = localtime (&now);

  // now calculate the moon phase by the timestamp
  uint8_t phase = astronomy.calculateMoonPhase(now);
  Serial.printf("Moon Phase: %s\n", MOON_PHASES[phase].c_str());

  // another option is to use year, month and day (in month)
  phase = astronomy.calculateMoonPhase(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  Serial.printf("Moon Phase: %s\n", MOON_PHASES[phase].c_str());

  // if you also need illumination there are two more methods which return a struct
  Astronomy::MoonData moonData = astronomy.calculateMoonData(now);
  Serial.printf("Moon Phase: %s\n", MOON_PHASES[moonData.phase].c_str());
  Serial.printf("Illumination: %f\n", moonData.illumination);

  moonData = astronomy.calculateMoonData(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  Serial.printf("Moon Phase: %s\n", MOON_PHASES[moonData.phase].c_str());
  Serial.printf("Illumination: %f\n", moonData.illumination);
}

void loop() {

}
