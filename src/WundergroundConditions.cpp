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
#include "WundergroundConditions.h"

WundergroundConditions::WundergroundConditions(boolean _isMetric) {
  isMetric = _isMetric;
}


void WundergroundConditions::updateConditions(WGConditions *conditions, String apiKey, String language, String country, String city) {
  doUpdate(conditions, "/api/" + apiKey + "/conditions/lang:" + language + "/q/" + country + "/" + city + ".json");
}

// wunderground change the API URL scheme:
// http://api.wunderground.com/api/<API-KEY>/conditions/lang:de/q/zmw:00000.215.10348.json
void WundergroundConditions::updateConditions(WGConditions *conditions, String apiKey, String language, String zmwCode) {
  doUpdate(conditions, "/api/" + apiKey + "/conditions/lang:" + language + "/q/zmw:" + zmwCode + ".json");
}

void WundergroundConditions::updateConditionsPWS(WGConditions *conditions, String apiKey, String language, String pws) {
  doUpdate(conditions, "/api/" + apiKey + "/conditions/lang:" + language + "/q/pws:" + pws + ".json");
}

void WundergroundConditions::doUpdate(WGConditions *conditions, String url) {
  this->conditions = conditions;
  JsonStreamingParser parser;
  parser.setListener(this);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("api.wunderground.com", httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.wunderground.com\r\n" +
               "Connection: close\r\n\r\n");
  int retryCounter = 0;
  while(!client.available()) {
    delay(1000);
    retryCounter++;
    if (retryCounter > 10) {
      return;
    }
  }

  int pos = 0;
  boolean isBody = false;
  char c;

  int size = 0;
  client.setNoDelay(false);
  while(client.connected()) {
    while((size = client.available()) > 0) {
      c = client.read();
      if (c == '{' || c == '[') {
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
      }
    }
  }
}

void WundergroundConditions::whitespace(char c) {
  Serial.println("whitespace");
}

void WundergroundConditions::startDocument() {
  Serial.println("start document");
}

void WundergroundConditions::key(String key) {
  currentKey = String(key);
}

void WundergroundConditions::value(String value) {

  if (currentKey == "wind_mph" && !isMetric) {
    conditions->windSpeed = value + "mph";
  }

  if (currentKey == "wind_kph" && isMetric) {
    conditions->windSpeed = value + "km/h";
  }

   if (currentKey == "wind_dir") {
    conditions->windDir = value;
  }

   if (currentKey == "local_time_rfc822") {
    conditions->date = value.substring(0, 16);
  }

  if (currentKey == "observation_time_rfc822") {
    conditions->observationDate = value.substring(0, 16);
  }

  if (currentKey == "observation_time") {
    conditions->observationTime = value;
  }


  if (currentKey == "temp_f" && !isMetric) {
    conditions->currentTemp = value;
  }
  if (currentKey == "temp_c" && isMetric) {
    conditions->currentTemp = value;
  }
  if (currentKey == "icon") {
    conditions->weatherIcon = value;
  }
  if (currentKey == "weather") {
    conditions->weatherText = value;
  }
  if (currentKey == "relative_humidity") {
    conditions->humidity = value;
  }
  if (currentKey == "pressure_mb" && isMetric) {
    conditions->pressure = value + "mb";
  }
  if (currentKey == "pressure_in" && !isMetric) {
    conditions->pressure = value + "in";
  }

  if (currentKey == "feelslike_f" && !isMetric) {
    conditions->feelslike = value;
  }

  if (currentKey == "feelslike_c" && isMetric) {
    conditions->feelslike = value;
  }

  if (currentKey == "UV") {
    conditions->UV = value;
  }

  if (currentKey == "dewpoint_f" && !isMetric) {
    conditions->dewPoint = value;
  }
  if (currentKey == "dewpoint_c" && isMetric) {
    conditions->dewPoint = value;
  }
  if (currentKey == "precip_today_metric" && isMetric) {
    conditions->precipitationToday = value + "mm";
  }
  if (currentKey == "precip_today_in" && !isMetric) {
    conditions->precipitationToday = value + "in";
  }

}

void WundergroundConditions::endArray() {

}


void WundergroundConditions::startObject() {
  currentParent = currentKey;
}

void WundergroundConditions::endObject() {
  currentParent = "";
}

void WundergroundConditions::endDocument() {

}

void WundergroundConditions::startArray() {

}


String WundergroundConditions::getMeteoconIcon(String iconText) {
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
