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

/* Customizations by Neptune (NeptuneEng on Twitter, Neptune2 on Github)
 *  
 *  Added Wifi Splash screen and credit to Squix78
 *  Modified progress bar to a thicker and symmetrical shape
 *  Replaced TimeClient with built-in lwip sntp client (no need for external ntp client library)
 *  Added Daylight Saving Time Auto adjuster with DST rules using simpleDSTadjust library
 *  https://github.com/neptune2/simpleDSTadjust
 *  Added Setting examples for Boston, Zurich and Sydney
  *  Selectable NTP servers for each locale
  *  DST rules and timezone settings customizable for each locale
   *  See https://www.timeanddate.com/time/change/ for DST rules
  *  Added AM/PM or 24-hour option for each locale
 *  Changed to 7-segment Clock font from http://www.keshikan.net/fonts-e.html
 *  Added Forecast screen for days 4-6 (requires 1.1.3 or later version of esp8266_Weather_Station library)
 *  Added support for DHT22, DHT21 and DHT11 Indoor Temperature and Humidity Sensors
 *  Fixed bug preventing display.flipScreenVertically() from working
 *  Slight adjustment to overlay
 *  Added support for 1-Wire DS18B20 sensors by nordis77 on GitHub, just have to figure out how to power them from ESP8266 NodeMcu
 *  Added support for uploding sensor data to Thingspeak.
 */

#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "settings.h"
#include <JsonListener.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <time.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "WundergroundClient.h"
#include "WeatherStationFonts.h"
#include "WeatherStationImages.h"
#include "DSEG7Classic-BoldFont.h"
#include "ThingspeakClient.h"


// Initialize Wunderground client with METRIC setting
WundergroundClient wunderground(IS_METRIC);

// Initialize the temperature/ humidity sensor
//DHT dht(DHTPIN, DHTTYPE);
//DS18B20 instead of DHT
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
//DS18B20 device Address, your addresses will be different
DeviceAddress indoorSensor = { 0x10, 0x47, 0xF5, 0x81, 0x1, 0x8, 0x0, 0xF4 };
DeviceAddress outdoorSensor = { 0x10, 0xF, 0x40, 0x37, 0x1, 0x8, 0x0, 0x25 };
DeviceAddress atticSensor = { 0x10, 0x8B, 0x44, 0x37, 0x1, 0x8, 0x0, 0xC };
DeviceAddress basementSensor = { 0x10, 0x6C, 0x4E, 0x37, 0x1, 0x8, 0x0, 0x1D };
//float humidity = 0.0; // For DHT
float indoor = 0.0;
float attic = 0.0;
float outdoor = 0.0;
float basement = 0.0;

ThingspeakClient thingspeak;

// flag changed in the ticker function every 10 minutes
bool readyForWeatherUpdate = false;
// flag changed in the ticker function every 1 minute
//bool readyForDHTUpdate = false;
bool readyForDS18B20Update = false;
// flag changed in the ticker function every ~10 minute
bool readyForThingSpeakUpdate = false;
String lastUpdate = "--";

Ticker ticker;

//declaring prototypes
void configModeCallback (WiFiManager *myWiFiManager);
void drawProgress(OLEDDisplay *display, int percentage, String label);
void drawOtaProgress(unsigned int, unsigned int);
void updateData(OLEDDisplay *display);
void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecast2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawIndoor(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawAtticBasement(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawThingspeak(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex);
void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void setReadyForWeatherUpdate();
int8_t getWifiQuality();
char indoorString[6];
char atticString[6];
char outdoorString[6];
char basementString[6];

// Add frames
// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
FrameCallback frames[] = { drawDateTime, drawCurrentWeather, drawIndoor, drawAtticBasement, drawThingspeak, drawForecast, drawForecast2  }; //add drawAtticBasement when ready
int numberOfFrames = 7; //number of Frames. Added a new frame for the added sensors

OverlayCallback overlays[] = { drawHeaderOverlay };
int numberOfOverlays = 1;

void setup() {
  // Turn On VCC
  // pinMode(D4, OUTPUT);
  // digitalWrite(D4, HIGH);
  Serial.begin(115200);

  // initialize display
  display.init();
  display.clear();
  display.display();
  
  //display.flipScreenVertically();  // Comment out to flip display 180deg
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setContrast(255);

  // Credit where credit is due
  display.drawXbm(-6, 5, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.drawString(88, 18, "Weather Station\nBy [nordis]");
  display.display();

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment for testing wifi manager
  // wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);

  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

  //Manual Wifi
  // WiFi.begin(SSID, PASSWORD);
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

    DS18B20.begin();
    DS18B20.setResolution(indoorSensor, 9);
    DS18B20.setResolution(outdoorSensor, 9);
    DS18B20.setResolution(atticSensor, 9);
    DS18B20.setResolution(basementSensor, 9);
  }

  ui.setTargetFPS(30);
  ui.setTimePerFrame(10*1000); // Setup frame display time to 10 sec
  
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

  // Setup OTA
  Serial.println("Hostname: " + hostname);
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.onProgress(drawOtaProgress);
  ArduinoOTA.begin();

  updateData(&display);

  ticker.attach(UPDATE_INTERVAL_SECS, setReadyForWeatherUpdate);
  ticker.attach(60, setReadyForDS18B20Update);
  ticker.attach(UPDATE_INTERVAL_SECS + 5, setReadyForThingSpeakUpdate); //UPDATE_INTEVAL_SECS = 10min
}

void loop() {

  if (readyForWeatherUpdate && ui.getUiState()->frameState == FIXED) {
    updateData(&display);
  }

  if (readyForDS18B20Update && ui.getUiState()->frameState == FIXED)
    updateTemp();

  if (readyForThingSpeakUpdate && ui.getUiState() ->frameState == FIXED)
    updateThingSpeak();
    
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
  display->drawProgressBar(2, 28, 124, 12, percentage);
  display->display();
}

void drawOtaProgress(unsigned int progress, unsigned int total) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, "OTA Update");
  display.drawProgressBar(2, 28, 124, 12, progress / (total / 100));
  display.display();
}

void updateData(OLEDDisplay *display) {
  drawProgress(display, 10, "Updating time...");
  configTime(UTC_OFFSET * 3600, 0, NTP_SERVERS);
  drawProgress(display, 30, "Updating conditions...");
  wunderground.updateConditions(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  drawProgress(display, 50, "Updating forecasts...");
  wunderground.updateForecast(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
////For DHT Sensor
//  drawProgress(display, 70, "Updating DHT Sensor");
//  humidity = dht.readHumidity();
//  drawProgress(display, 80, "Updating DHT Sensor");
//  temperature = dht.readTemperature(!IS_METRIC);
//  delay(500);

  drawProgress(display, 65, "Updating Sensors...");
  DS18B20.requestTemperatures();
  delay(1000); //These delays may or may not be needed
  do {
    indoor = DS18B20.getTempC(indoorSensor);
    delay(1000);
  }while (indoor == 85.00 || indoor == (-127.00));
    do {
    attic = DS18B20.getTempC(atticSensor); //attic
    delay(1000);
  } while (attic == 85.00 || attic == (-127.00));
  do {
    outdoor = DS18B20.getTempC(outdoorSensor); //outdoor
    delay(1000);
  } while (outdoor == 85.00 || outdoor == (-127.00));
  do {
    basement = DS18B20.getTempC(basementSensor);
    delay(1000);
  } while (basement == 85.00 || basement == (-127.00));
  delay(200); //sensors need 750mS to read
  
  drawProgress(display, 90, "Updating thingspeak...");
  thingspeak.getLastChannelItem(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_READ_KEY);
  readyForWeatherUpdate = false;
  drawProgress(display, 100, "Done...");
  delay(200);
}

//// Called every 1 minute
//void updateDHT() {
//  humidity = dht.readHumidity();
//  temperature = dht.readTemperature(!IS_METRIC);
//  readyForDHTUpdate = false;
//}

//Call every 1 minute
void updateTemp() {    
  DS18B20.requestTemperatures();
  delay(1000); //These delays may or may not be needed
  do {
    indoor = DS18B20.getTempC(indoorSensor); //indoor
    delay(1000);
  } while (indoor == 85.00 || indoor == (-127.00));
  do {
    attic = DS18B20.getTempC(atticSensor); //attic
    delay(1000);
  } while (attic == 85.00 || attic == (-127.00));
  do {;
    outdoor = DS18B20.getTempC(outdoorSensor); //outdoor
    delay(1000);
  } while (outdoor == 85.00 || outdoor == (-127.00));
  do {
    basement = DS18B20.getTempC(atticSensor);
    delay(1000);
  } while (basement == 85.00 || basement == (-127.00));
  delay(200);
  readyForDS18B20Update = false;
}

void updateThingSpeak() {
  dtostrf(indoor, 2, 2, indoorString);
  dtostrf(outdoor, 2, 2, outdoorString);
  dtostrf(attic, 2, 2, atticString);
  dtostrf(basement, 2, 2, basementString);

  WiFiClient client;
  //Serial.println("Connecting to ThingSpeak");
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    //Serial.println("Connetion failed!");
    return;
  }
  client.print(String("GET ") + path + "&field1=" + indoorString +
              "&field2=" + atticString +
              "&field3=" + outdoorString +
              "&field4=" + basementString +
              " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "Connection: keep-alive\r\n\r\n");
  delay(500);
  readyForThingSpeakUpdate = false;
}

void drawDateTime(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  char *dstAbbrev;
  char time_str[11];
  time_t now = dstAdjusted.time(&dstAbbrev);
  struct tm * timeinfo = localtime (&now);
  
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  String date = ctime(&now);
  date = date.substring(0,11) + String(1900+timeinfo->tm_year);
  int textWidth = display->getStringWidth(date);
  display->drawString(64 + x, 5 + y, date);
  display->setFont(DSEG7_Classic_Bold_21);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  
#ifdef STYLE_24HR
  sprintf(time_str, "%02d:%02d:%02d\n",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  display->drawString(108 + x, 19 + y, time_str);
#else
  int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
  sprintf(time_str, "%2d:%02d:%02d\n",hour, timeinfo->tm_min, timeinfo->tm_sec);
  display->drawString(101 + x, 19 + y, time_str);
#endif

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
#ifdef STYLE_24HR
  sprintf(time_str, "%s", dstAbbrev);
  display->drawString(108 + x, 27 + y, time_str);  // Known bug: Cuts off 4th character of timezone abbreviation
#else
  sprintf(time_str, "%s\n%s", dstAbbrev, timeinfo->tm_hour>=12?"pm":"am");
  display->drawString(102 + x, 18 + y, time_str);
#endif

}

void drawCurrentWeather(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(60 + x, 5 + y, wunderground.getWeatherText());

  display->setFont(ArialMT_Plain_24);
  String temp = wunderground.getCurrentTemp() + (IS_METRIC ? "°C": "°F");
  
  display->drawString(60 + x, 15 + y, temp);
  int tempWidth = display->getStringWidth(temp);

  display->setFont(Meteocons_Plain_42);
  String weatherIcon = wunderground.getTodayIcon();
  int weatherIconWidth = display->getStringWidth(weatherIcon);
  display->drawString(32 + x - weatherIconWidth / 2, 05 + y, weatherIcon);
}


void drawForecast(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  drawForecastDetails(display, x, y, 0);
  drawForecastDetails(display, x + 44, y, 2);
  drawForecastDetails(display, x + 88, y, 4);
}

void drawForecast2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  drawForecastDetails(display, x, y, 6);
  drawForecastDetails(display, x + 44, y, 8);
  drawForecastDetails(display, x + 88, y, 10);
}

void drawIndoor(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0, "1-Wire Sensors" );
  display->setFont(ArialMT_Plain_16);
  dtostrf(indoor,4, 1, FormattedIndoor);
  display->drawString(64+x, 12, "In: " + String(FormattedIndoor) + (IS_METRIC ? "°C": "°F"));
  dtostrf(outdoor,4, 1, FormattedOutdoor);
  display->drawString(64+x, 30, "Out: " + String(FormattedOutdoor) + (IS_METRIC ? "°C": "°F"));
}

void drawAtticBasement(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0, "1-Wire Sensors" );
  display->setFont(ArialMT_Plain_16);
  dtostrf(attic,4, 1, FormattedAttic);
  display->drawString(64+x, 12, "Attic: " + String(FormattedAttic) + (IS_METRIC ? "°C": "°F"));
  dtostrf(outdoor,4, 1, FormattedBasement);
  display->drawString(64+x, 30, "Basement: " + String(FormattedBasement) + (IS_METRIC ? "°C": "°F"));
}

void drawThingspeak(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64 + x, 0 + y, "Thingspeak Sensors");
  display->setFont(ArialMT_Plain_16);
  display->drawString(64 + x, 12 + y, "In: " + thingspeak.getFieldValue(0) + "°C");
  // display->drawString(64 + x, 12 + y, thingspeak.getFieldValue(0) + (IS_METRIC ? "°C": "°F"));  // Needs code to convert Thingspeak temperature string
  display->drawString(64 + x, 30 + y, "Out: " + thingspeak.getFieldValue(2) + "°C");
}

void drawForecastDetails(OLEDDisplay *display, int x, int y, int dayIndex) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  String day = wunderground.getForecastTitle(dayIndex).substring(0, 3);
  day.toUpperCase();
  display->drawString(x + 20, y, day);

  display->setFont(Meteocons_Plain_21);
  display->drawString(x + 20, y + 12, wunderground.getForecastIcon(dayIndex));

  display->setFont(ArialMT_Plain_10);
  display->drawString(x + 20, y + 34, wunderground.getForecastLowTemp(dayIndex) + "|" + wunderground.getForecastHighTemp(dayIndex));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  char time_str[11];
  time_t now = dstAdjusted.time(nullptr);
  struct tm * timeinfo = localtime (&now);

  display->setFont(ArialMT_Plain_10);

#ifdef STYLE_24HR
  sprintf(time_str, "%02d:%02d:%02d\n",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#else
  int hour = (timeinfo->tm_hour+11)%12+1;  // take care of noon and midnight
  sprintf(time_str, "%2d:%02d:%02d%s\n",hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_hour>=12?"pm":"am");
#endif

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(5, 52, time_str);

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  String temp = wunderground.getCurrentTemp() + (IS_METRIC ? "°C": "°F");
  display->drawString(101, 52, temp);

  int8_t quality = getWifiQuality();
  for (int8_t i = 0; i < 4; i++) {
    for (int8_t j = 0; j < 2 * (i + 1); j++) {
      if (quality > i * 25 || j == 0) {
        display->setPixel(120 + 2 * i, 61 - j);
      }
    }
  }

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(Meteocons_Plain_10);
  String weatherIcon = wunderground.getTodayIcon();
  int weatherIconWidth = display->getStringWidth(weatherIcon);
  // display->drawString(64, 55, weatherIcon);
  display->drawString(77, 53, weatherIcon);

  display->drawHorizontalLine(0, 51, 128);
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

void setReadyForWeatherUpdate() {
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;
}

//void setReadyForDHTUpdate() {
//  Serial.println("Setting readyForDHTUpdate to true");
//  readyForDHTUpdate = true;
//}
void setReadyForDS18B20Update() {
  Serial.println("Setting readyForDS18B20Update to true");
  readyForDS18B20Update = true;
}

void setReadyForThingSpeakUpdate() {
  Serial.println("Setting setReadyForThingSpeakUpdate to true");
  readyForThingSpeakUpdate = true;
}

