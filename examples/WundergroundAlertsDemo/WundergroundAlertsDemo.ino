/**The MIT License (MIT)

Copyright (c) 2017 by Daniel Eichhorn

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

See more at https://blog.squix.org
*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <JsonListener.h>
#include "WundergroundAlerts.h"

/**
 * Wunderground Settings
 */
const String  WUNDERGRROUND_API_KEY = "<Wunderground-KEY>";
const String  WUNDERGR_UND_STATE_OR_COUNTRY = "CH";
const String  WUNDERGR_UND_CITY = "ZURICH";
const String  WUNDERGRROUND_LANGUAGE = "EN";
const boolean IS_METRIC = true;
const boolean USE_PM = false;

// initiate the WundergoundClient
WundergroundAlerts wunderground;


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
  uint8_t maxAlerts = 3;
  WGAlert alerts[maxAlerts];
  wunderground.updateAlerts(alerts, maxAlerts, WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGR_UND_STATE_OR_COUNTRY, WUNDERGR_UND_CITY);

  for (int i = 0; i < wunderground.getActiveAlertsCnt(); i++) {
    Serial.println("------------------------------------");
    Serial.println("activeAlerts: " + alerts[i].activeAlerts);
    Serial.println("activeAlertsMessage: " + alerts[i].activeAlertsMessage);
    Serial.println("activeAlertsMessageTrunc: " + alerts[i].activeAlertsMessageTrunc);
    Serial.println("activeAlertsText: " + alerts[i].activeAlertsText);
    Serial.println("activeAlertsStart: " + alerts[i].activeAlertsStart);
    Serial.println("activeAlertsEnd: " + alerts[i].activeAlertsEnd);
    Serial.println("activeAlertsPhenomena: " + alerts[i].activeAlertsPhenomena);
    Serial.println("activeAlertsSignificance: " + alerts[i].activeAlertsSignificance);
    Serial.println("activeAlertsAttribution: " + alerts[i].activeAlertsAttribution);
  }


  Serial.println();
  Serial.println("---------------------------------------------------/\n");

}


/**
 * LOOP
 */
void loop() {

}
