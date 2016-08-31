/**The MIT License (MIT)

Copyright (c) 2016 by Daniel Eichhorn

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

See more at http://blog.squix.ch
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <JsonListener.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//#include "SSD1306Wire.h"
#include "SH1106Wire.h"
#include "OLEDDisplayUi.h"
#include "Wire.h"
#include "images.h"

#include "TimeClient.h"
#include "AdsbExchangeClient.h"

/**************
 * Required Libraries:
 * - Weather Station by Daniel Eichhorn 
 * - WifiManager by tzapu
 * - ESP8266 OLED Driver by Daniel Eichhorn, Fabrice Weinberg
 * - Json Streaming Parser by Daniel Eichhorn
 */


/***************************
 * Begin Settings
 **************************/
// Please read http://blog.squix.org/weatherstation-getting-code-adapting-it
// for setup instructions

#define HOSTNAME "ESP8266-OTA-"

// Setup
const int UPDATE_INTERVAL_SECS_LONG = 15; // Update every 15 seconds if no airplanes around
const int UPDATE_INTERVAL_SECS_SHORT = 3; // Update every 3 seconds if there are airplanes

int currentUpdateInterval = UPDATE_INTERVAL_SECS_LONG;
long lastUpdate = 0;

// Check http://www.virtualradarserver.co.uk/Documentation/Formats/AircraftList.aspx
// to craft this query to your needs
const String QUERY_STRING = "lat=47.424341887&lng=8.568778038&fDstL=0&fDstU=10&fAltL=0&fAltL=1500&fAltU=10000";

const int UTC_OFFSET = 2;

const float pi = 3.141;

// Display Settings
const int I2C_DISPLAY_ADDRESS = 0x3c;
const int SDA_PIN = D3;
const int SDC_PIN = D4;

// Initialize the oled display for address 0x3c
// sda-pin=14 and sdc-pin=12
//SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
SH1106Wire      display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);
OLEDDisplayUi   ui( &display );

/***************************
 * End Settings
 **************************/

TimeClient timeClient(UTC_OFFSET);

AdsbExchangeClient adsbClient;

// flag changed in the ticker function every 10 minutes
bool readyForUpdate = false;

Ticker ticker;

//declaring prototypes
void configModeCallback (WiFiManager *myWiFiManager);
void drawProgress(OLEDDisplay *display, int percentage, String label);
void drawOtaProgress(unsigned int, unsigned int);
void updateData(OLEDDisplay *display);
void drawCurrentAirplane1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentAirplane2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentAirplane3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void drawTextAsBigAsPossible(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y, String text, int maxWidth);
void drawHeading(OLEDDisplay *display, int x, int y, double heading);
void checkReadyForUpdate();
int8_t getWifiQuality();


// Add frames
// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
FrameCallback frames[] = { drawCurrentAirplane1, drawCurrentAirplane2, drawCurrentAirplane3 };
int numberOfFrames = 3;

OverlayCallback overlays[] = { drawHeaderOverlay };
int numberOfOverlays = 1;

void setup() {
  Serial.begin(115200);

  // initialize dispaly
  display.init();
  display.clear();
  display.display();

  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  // Uncomment for testing wifi manager
  //wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);

  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

  //Manual Wifi
  //WiFi.begin(WIFI_SSID, WIFI_PWD);
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);


  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.clear();
    display.drawString(64, 10, "Connecting to WiFi");
    display.drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbol : inactiveSymbol);
    display.drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbol : inactiveSymbol);
    display.drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbol : inactiveSymbol);
    display.display();

    counter++;
  }

  ui.setTargetFPS(30);

  //Hack until disableIndicator works:
  //Set an empty symbol
  ui.setActiveSymbol(emptySymbol);
  ui.setInactiveSymbol(emptySymbol);
  ui.disableIndicator();

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  ui.setFrames(frames, numberOfFrames);

  ui.setOverlays(overlays, numberOfOverlays);

  // Inital UI takes care of initalising the display too.
  ui.init();

  // Setup OTA
  Serial.println("Hostname: " + hostname);
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.onProgress(drawOtaProgress);
  ArduinoOTA.begin();

  updateData(&display);

  //Check every second
  ticker.attach(1, checkReadyForUpdate);

}

void loop() {
  // If there are airplanes query often
  if (adsbClient.getNumberOfVisibleAircrafts() == 0) {
    currentUpdateInterval = UPDATE_INTERVAL_SECS_LONG;
  } else {
    currentUpdateInterval = UPDATE_INTERVAL_SECS_SHORT;
  }

  if (readyForUpdate && ui.getUiState()->frameState == FIXED) {
    updateData(&display);
  }

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    ArduinoOTA.handle();
    delay(remainingTimeBudget);
  }


}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, "Wifi Manager");
  display.drawString(64, 20, "Please connect to AP");
  display.drawString(64, 30, myWiFiManager->getConfigPortalSSID());
  display.drawString(64, 40, "To setup Wifi Configuration");
  display.display();
}

void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}

void drawOtaProgress(unsigned int progress, unsigned int total) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, "OTA Update");
  display.drawProgressBar(2, 28, 124, 10, progress / (total / 100));
  display.display();
}

void updateData(OLEDDisplay *display) {
  readyForUpdate = false;
  adsbClient.updateVisibleAircraft(QUERY_STRING);
  lastUpdate = millis();
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {

  if (adsbClient.isAircraftVisible()) {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_10);
    display->drawString(0, 10, "Dst:");
    display->drawString(64, 10, "Alt:");
    display->drawString(0, 32, "Head:");
    display->setFont(ArialMT_Plain_16);
    display->drawString(0, 20, String(adsbClient.getDistance()) + "km");
    display->drawString(64, 20, adsbClient.getAltitude() + "ft");
    display->drawString(0, 42, String(adsbClient.getHeading()) + "°");

    drawHeading(display, 78, 52, adsbClient.getHeading());

  }

  int8_t quality = getWifiQuality();
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 2 * (i + 1); j++) {
      if (quality > i * 25 || j == 0) {
        display->setPixel(120 + 2 * i, 63 - j);
      }
    }
  }

}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  if(dbm <= -100) {
      return 0;
  } else if(dbm >= -50) {
      return 100;
  } else {
      return 2 * (dbm + 100);
  }
}

void drawCurrentAirplane1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if (adsbClient.isAircraftVisible()) {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_10);

    display->drawString(0 + x, 0 + y, "From: " + adsbClient.getFrom());
  }
}

void drawCurrentAirplane2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if (adsbClient.isAircraftVisible()) {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_10);

    display->drawString(0 + x, 0 + y, "To: " + adsbClient.getTo());
  }
}

void drawCurrentAirplane3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  if (adsbClient.isAircraftVisible()) {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_10);

    display->drawString(0 + x, 0 + y, "Type: " + adsbClient.getAircraftType());
  }
}

void drawHeading(OLEDDisplay *display, int x, int y, double heading) {
  int degrees[] = {0, 170, 190, 0};
  display->drawCircle(x, y, 10);
  int radius = 8;
  for (int i = 0; i < 3; i++) {
    int x1 = cos((-450 + (heading + degrees[i])) * pi / 180.0) * radius + x;
    int y1 = sin((-450 + (heading + degrees[i])) * pi / 180.0) * radius + y;
    int x2 = cos((-450 + (heading + degrees[i + 1])) * pi / 180.0) * radius + x;
    int y2 = sin((-450 + (heading + degrees[i + 1])) * pi / 180.0) * radius + y;
    display->drawLine(x1, y1, x2, y2);

  }
}

void checkReadyForUpdate() {
  // Only do light work in ticker callback
  if (lastUpdate < millis() - currentUpdateInterval * 1000) {
      readyForUpdate = true;
  }
}
