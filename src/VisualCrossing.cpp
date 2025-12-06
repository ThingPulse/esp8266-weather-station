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

#include <ESPWiFi.h>
#include <WiFiClientSecure.h>
#include "VisualCrossing.h"

VisualCrossing::VisualCrossing() {

}

uint8_t VisualCrossing::updateForecasts(VisualCrossingData *forecastData, VisualCrossingData &currentData, String key, String location, uint8_t maxForecasts) {
  this->maxForecasts = maxForecasts;
  return doUpdate(forecastData, currentData, buildPath(key, location));
}

String VisualCrossing::buildPath(String key, String locationParameter) {
  String units = metric ? "metric" : "us";
  return "/VisualCrossingWebServices/rest/services/timeline/" + locationParameter + "/next" + (maxForecasts-1) + "days?"  + "unitGroup=" + units + "&lang=" + language + "&include=days,fcst,current&iconset=icons2" + "&key=" + key + "&contentType=json";
}

uint8_t VisualCrossing::doUpdate(VisualCrossingData *forecastData, VisualCrossingData &currentData, String path) {
  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();
  this->currentForecast = 0;
  this->forecastData = forecastData;
  this->currentData = &currentData;
  JsonStreamingParser parser;
  parser.setListener(this);
  Serial.printf("[HTTPS] Requesting resource at https://%s:%u%s\n", host.c_str(), port, path.c_str());

  WiFiClientSecure client;
  client.setInsecure();
  if(client.connect(host.c_str(), port)) {
    bool isBody = false;
    char c;
    Serial.println("[HTTPS] connected, now GETting data");
    client.print("GET " + path + " HTTP/1.1\r\n"
                 "Host: " + host + "\r\n"
                 "Connection: close\r\n\r\n");

    while (client.connected() || client.available()) {
      if (client.available()) {
        if ((millis() - lost_do) > lostTest) {
          Serial.println("[HTTPS] lost in client with a timeout");
          client.stop();
          ESP.restart();
        }
        c = client.read();
        if (c == '{' || c == '[') {
          isBody = true;
        }
        if (isBody) {
          parser.parse(c);
        }
      }
      // give WiFi and TCP/IP libraries a chance to handle pending events
      yield();
    }
    client.stop();
  } else {
    Serial.println("[HTTPS] failed to connect to host");
  }
  Serial.println(currentData.description);
  this->forecastData = nullptr;
  this->currentData = nullptr;
  return currentForecast;
}

void VisualCrossing::whitespace(char c) {
  Serial.println("whitespace");
}

void VisualCrossing::startDocument() {
  Serial.println("start document");
}

void VisualCrossing::key(String key) {
  currentKey = String(key);
}

void VisualCrossing::value(String value) {
  if (currentParent == "") {
      // "latitude": 47.37 float latitude;
      if (currentKey == "latitude") {
        this->currentData->latitude = value.toFloat();
      }
      // "longitude": 8.54, float longitude;
      if (currentKey == "longitude") {
        this->currentData->longitude = value.toFloat();
      }
  }
  else if (currentParent == "days" || currentParent == "currentConditions") {
    VisualCrossingData *inProgressData;
    if (currentParent == "days") inProgressData = &forecastData[currentForecast];
    else if (currentParent == "currentConditions") inProgressData = currentData;
	else return;
  
    // "datetime": "2018-05-23", String observationTimeText;
    if (currentKey == "datetime") {
      inProgressData->observationTimeText = value;
    }
    // {"datetimeEpoch":1527066000,  uint32_t observationTime;
    if (currentKey == "datetimeEpoch") {
      inProgressData->observationTime = value.toInt();
    }
    // "temp":17.35, float temp;
    if (currentKey == "temp") {
      inProgressData->temp = value.toFloat();
    }
    //   "tempmin":16.89, float tempMin;
    if (currentKey == "tempmin") {
      inProgressData->tempMin = value.toFloat();
    }
    //   "temp_max":17.35, float tempMax;
    if (currentKey == "tempmax") {
      inProgressData->tempMax = value.toFloat();
    }
    //   "humidity":97, float humidity;
    if (currentKey == "humidity") {
      inProgressData->humidity = value.toFloat();
    }
    //   "windspeed":1.77, float windSpeed;
    if (currentKey == "windspeed") {
      inProgressData->windSpeed = value.toFloat();
    }
    //   "deg":207.501 float windDeg;
    if (currentKey == "winddir") {
      inProgressData->windDeg = value.toFloat();
    }
    //   "pressure":970.8, float pressure;
    if (currentKey == "pressure") {
      inProgressData->pressure = value.toFloat();
    }
    //   "cloudcover": 44.4, float clouds;
    if (currentKey == "cloudcover") {
      inProgressData->clouds = value.toFloat();
    }
    //   "sunriseEpoch":1526960448, uint32_t sunrise;
    if (currentKey == "sunriseEpoch") {
      inProgressData->sunrise = value.toInt();
    }
    //   "sunsetEpoch": 1527015901, uint32_t sunset;
    if (currentKey == "sunsetEpoch") {
      inProgressData->sunset = value.toInt();
    }
    //   "conditions":"Partially cloudy", String conditions;
    if (currentKey == "conditions") {
      inProgressData->conditions = value;
    }
    //   "description":"Partly cloudy throughout the day.", String description;
    if (currentKey == "description") {
      inProgressData->description = value;
    }
    //   "icon":"partly-cloudy-day", String icon;
    if (currentKey == "icon") {
      inProgressData->icon = value;
    }
  }
}

void VisualCrossing::endArray() {

}


void VisualCrossing::startObject() {
  if (currentKey ==  "days" || currentKey == "stations" || currentKey == "currentConditions")
    currentParent = currentKey;
}

void VisualCrossing::endObject() {
  if (currentParent == "days") {
    // Always use daytime MeteoCon for forecast
	forecastData[currentForecast].iconMeteoCon = getMeteoconIcon(forecastData[currentForecast].icon);
    currentForecast++;
    if (currentForecast >= maxForecasts) currentParent = "";
  }
  else if (currentParent == "currentConditions") {
    currentParent = "";
    // Get MeteoCon after times are available
	bool night = currentData->sunrise && currentData->sunset &&
                 currentData->observationTime && 
                (currentData->observationTime < currentData->sunrise ||
                 currentData->observationTime > currentData->sunset);
    currentData->iconMeteoCon = getMeteoconIcon(currentData->icon, night);
  }
}

void VisualCrossing::endDocument() {

}

void VisualCrossing::startArray() {

}


String VisualCrossing::getMeteoconIcon(String icon, bool night) {
  if (icon == "clear-day") return "B";
  if (icon == "clear-night") return "C";
  if (icon == "partly-cloudy-day") return "H";
  if (icon == "partly-cloudy-night") return "4";
  if (icon == "cloudy") {
    if (!night) return "N";
    else return "5";
  }
  if (icon == "wind") return "F";
  if (icon == "fog") return "M";
  if (icon == "showers-day") return "Q";
  if (icon == "showers-night") return "7";
  if (icon == "rain") {
    if (!night) return "R";
    else return "8";
  }
  if (icon == "thunder-showers-day") return "P";
  if (icon == "thunder-showers-night") return "6";
  if (icon == "thunder-rain") {
    if (!night) return "O";
    else return "&";
  }
  if (icon == "snow-showers-day") return "U";
  if (icon == "snow-showers-night") return "\"";
  if (icon == "snow") {
    if (!night) return "W";
    else return "#";
  }
  // Nothing matched: N/A
  return ")";
}
