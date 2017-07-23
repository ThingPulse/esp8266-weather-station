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

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "WundergroundHourly.h"

WundergroundHourly::WundergroundHourly(boolean _isMetric, boolean _is24Hours) {
  isMetric = _isMetric;
  is24Hours = _is24Hours;
}

void WundergroundHourly::setMetric(bool isMetric) {
  this->isMetric = isMetric;
}
void WundergroundHourly::set24Hours(bool is24Hours) {
  this->is24Hours = is24Hours;
}

void WundergroundHourly::updateHourly(WGHourly *hourlies, String apiKey, String language, String country, String city) {
  doUpdate(hourlies, "http://api.wunderground.com/api/" + apiKey + "/hourly/lang:" + language + "/q/" + country + "/" + city + ".json");
}


void WundergroundHourly::updateHourlyPWS(WGHourly *hourlies, String apiKey, String language, String pws) {
  doUpdate(hourlies, "http://api.wunderground.com/api/" + apiKey + "/hourly/lang:" + language + "/q/pws:" + pws + ".json");
}

void WundergroundHourly::updateHourlyZMW(WGHourly *hourlies, String apiKey, String language, String zmwCode) {
  doUpdate(hourlies, "http://api.wunderground.com/api/" + apiKey + "/hourly/lang:" + language + "/q/zmw:" + zmwCode + ".json");
}

void WundergroundHourly::doUpdate(WGHourly *hourlies, String url) {
  this->hourlies = hourlies;
  JsonStreamingParser parser;
  parser.setListener(this);

  HTTPClient http;

  http.begin(url);
  bool isBody = false;
  char c;
  int size;
  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  if(httpCode > 0) {



    WiFiClient * client = http.getStreamPtr();

    while(client->connected()) {
      while((size = client->available()) > 0) {
        c = client->read();
        if (c == '{' || c == '[') {

          isBody = true;
        }
        if (isBody) {
          parser.parse(c);
        }
      }
    }
  }
  this->hourlies = nullptr;
}

void WundergroundHourly::whitespace(char c) {
  Serial.println("whitespace");
}

void WundergroundHourly::startDocument() {
  Serial.println("start document");
}

void WundergroundHourly::key(String key) {
  currentKey = String(key);

}

void WundergroundHourly::value(String value) {
  if (currentKey == "hour") {
    currentHour = value.toInt();
  }

  if (is24Hours && currentKey == "hour_padded") {
    hourlies[currentHour].hour = value + ":00";
  }
  if (!is24Hours && currentKey == "civil") {
    hourlies[currentHour].hour = value;
  }

  if (currentKey == "icon") {
    hourlies[currentHour].icon = value;
  }
  if (currentKey == "condition") {
    hourlies[currentHour].title = value;
  }


  if (currentParent == "temp" && currentKey == "english" && !isMetric) {
    hourlies[currentHour].temp = value;
  }
  if (currentParent == "temp" && currentKey == "metric" && isMetric) {
    hourlies[currentHour].temp = value;
  }

  if (currentKey == "pop") {
      hourlies[currentHour].PoP = value;
  }

}

void WundergroundHourly::endArray() {

}


void WundergroundHourly::startObject() {
  currentParent = currentKey;
}

void WundergroundHourly::endObject() {
  currentParent = "";
}

void WundergroundHourly::endDocument() {

}

void WundergroundHourly::startArray() {

}



String WundergroundHourly::getMeteoconIcon(String iconText) {
  if (iconText == "chanceflurries") return "F";
  if (iconText == "chancerain") return "Q";
  if (iconText == "chancesleet") return "W";
  if (iconText == "chancesnow") return "V";
  if (iconText == "chancetstorms") return "S";
  if (iconText == "clear") return "B";
  if (iconText == "cloudy") return "Y";
  if (iconText == "flurries") return "F";
  if (iconText == "fog") return "M";
  if (iconText == "hazy") return "E";
  if (iconText == "mostlycloudy") return "Y";
  if (iconText == "mostlysunny") return "H";
  if (iconText == "partlycloudy") return "H";
  if (iconText == "partlysunny") return "J";
  if (iconText == "sleet") return "W";
  if (iconText == "rain") return "R";
  if (iconText == "snow") return "W";
  if (iconText == "sunny") return "B";
  if (iconText == "tstorms") return "0";

  if (iconText == "nt_chanceflurries") return "F";
  if (iconText == "nt_chancerain") return "7";
  if (iconText == "nt_chancesleet") return "#";
  if (iconText == "nt_chancesnow") return "#";
  if (iconText == "nt_chancetstorms") return "&";
  if (iconText == "nt_clear") return "2";
  if (iconText == "nt_cloudy") return "Y";
  if (iconText == "nt_flurries") return "9";
  if (iconText == "nt_fog") return "M";
  if (iconText == "nt_hazy") return "E";
  if (iconText == "nt_mostlycloudy") return "5";
  if (iconText == "nt_mostlysunny") return "3";
  if (iconText == "nt_partlycloudy") return "4";
  if (iconText == "nt_partlysunny") return "4";
  if (iconText == "nt_sleet") return "9";
  if (iconText == "nt_rain") return "7";
  if (iconText == "nt_snow") return "#";
  if (iconText == "nt_sunny") return "4";
  if (iconText == "nt_tstorms") return "&";

  return ")";
}
