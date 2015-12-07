/**The MIT License (MIT)

Copyright (c) 2015 by Daniel Eichhorn

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


#include <JsonListener.h>
#include <ESP8266WiFi.h>
#include "ssd1306_i2c.h"
#include "Wire.h"
#include "WundergroundClient.h"
#include "fonts.h"
#include <Ticker.h>
#include "NTPClient.h"

// Initialize the oled display for address 0x3c
// sda-pin=14 and sdc-pin=12
SSD1306 display(0x3c, D6, D5);
//SSD1306 display(0x3c, 0, 2);

// Set to false, if you prefere imperial/inches, Fahrenheit
WundergroundClient wunderground(true);

int utcOffset = 1;
NTPClient ntpClient(utcOffset);

// Add your wounderground api key here
String apiKey = "<wundergroundapikey>";
String country = "CH";
String city = "Zurich";

// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
void (*frameCallbacks[])(int x, int y) = {drawFrame1, drawFrame2, drawFrame3, drawFrame4};

int numberOfFrames = 4;

// flag changed in the ticker function every 10 minutes
bool readyForWeatherUpdate = false;

String lastUpdate = "--";

Ticker ticker;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  
  // initialize dispaly
  display.init();
  display.clear();
  display.display();
  WiFi.begin("yourssid", "yourpassw0rd");
  
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
  }
  
  ntpClient.begin();

  display.setFrameCallbacks(numberOfFrames, frameCallbacks);
  // how many ticks does a slide of frame take?
  display.setFrameTransitionTicks(10);
  // how many ticks should we wait until the next transition begins?
  display.setFrameWaitTicks(150);
  
  Serial.println("");
  
  wunderground.updateConditions(apiKey, country, city);
  wunderground.updateForecast(apiKey, country, city);
  lastUpdate = ntpClient.getFormattedTime();
  
  ticker.attach(10 * 60, setReadyForWeatherUpdate);

}

void loop() {
  if (readyForWeatherUpdate && display.getFrameState() == display.FRAME_STATE_FIX) {
    ntpClient.update();
    wunderground.updateConditions(apiKey, country, city);
    wunderground.updateForecast(apiKey, country, city);
    readyForWeatherUpdate = false;
    lastUpdate = ntpClient.getFormattedTime();
  }

  Serial.println(lastUpdate);
  
  //display.clear();

  display.clear();
  display.nextFrameTick();
  display.display();

  //delay(1000);

}

void drawFrame1(int x, int y) {
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  String date = wunderground.getDate();
  int textWidth = display.getStringWidth(date);
  display.drawString(64 + x, 10 + y, date);
  display.setFont(ArialMT_Plain_24);
  String time = ntpClient.getFormattedTime();
  textWidth = display.getStringWidth(time);
  display.drawString(64 + x, 20 + y, time);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawFrame2(int x, int y) {
  display.setFont(ArialMT_Plain_10);
  display.drawString(64 + x, 10 + y, wunderground.getWeatherText());
  
  display.setFont(ArialMT_Plain_24);
  String temp = wunderground.getCurrentTemp();
  display.drawString(64 + x, 20 + y, temp);
  int tempWidth = display.getStringWidth(temp);
  
  display.setFont(Meteocons_0_42);
  String weatherIcon = wunderground.getTodayIcon();
  int weatherIconWidth = display.getStringWidth(weatherIcon);
  display.drawString(32 + x - weatherIconWidth / 2, 10 + y, weatherIcon);
  display.drawString(64 + tempWidth + x, 15 + y, "*");
}

void drawFrame3(int x, int y) {
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(32 + x, 0 + y, "Humidity");
  display.drawString(96 + x, 0 + y, "Pressure");
  display.drawString(32 + x, 28 + y, "Precipit.");  
  
  display.setFont(ArialMT_Plain_16);
  display.drawString(32 + x, 10 + y, wunderground.getHumidity());
  display.drawString(96 + x, 10 + y, wunderground.getPressure());
  display.drawString(32 + x, 38 + y, wunderground.getPrecipitationToday());
  // display.drawRect(64 + x, 0 + y, 0, 54);
  // display.drawRect(0 + x, 27 + y, 128, 0);
  //display.drawString(64 + x,  + y, lastUpdate);
}
void drawFrame4(int x, int y) {
  drawForecast(x, y, 0);
  drawForecast(x + 44, y, 2);
  drawForecast(x + 88, y, 4);
}
void drawFrame5(int x, int y) {
  drawForecast(x, y, 2);
}
void drawFrame6(int x, int y) {
  drawForecast(x, y, 4);
}

void drawForecast(int x, int y, int dayIndex) {
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  String day = wunderground.getForecastTitle(dayIndex).substring(0, 3);
  day.toUpperCase();
  display.drawString(x + 20, y, day);
  
  display.setFont(Meteocons_0_21);
  display.drawString(x + 20, y + 15, wunderground.getForecastIcon(dayIndex));

  display.setFont(ArialMT_Plain_16);
  display.drawString(x + 20, y + 37, wunderground.getForecastLowTemp(dayIndex) + "/" + wunderground.getForecastHighTemp(dayIndex));
  //display.drawString(x + 20, y + 51, );
  display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void drawFrame7(int x, int y) {

}

void setReadyForWeatherUpdate() {
  Serial.println("Setting readyForUpdate to true");
  readyForWeatherUpdate = true;  
}



