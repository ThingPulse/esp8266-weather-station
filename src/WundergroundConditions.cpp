/**The MIT License (MIT)

Copyright (c) 2018 by Daniel Eichhorn, ThingPulse

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

#include <ESPWiFi.h>
#include <WiFiClient.h>
#include <ESPHTTPClient.h>
#include "WundergroundConditions.h"

WundergroundConditions::WundergroundConditions(boolean _isMetric) {
  isMetric = _isMetric;
}

void WundergroundConditions::setMetric(boolean isMetric) {
  this->isMetric = isMetric;
}

void WundergroundConditions::updateConditions(WGConditions *conditions, String apiKey, String language, String country, String city) {
  doUpdate(conditions, "http://api.wunderground.com/api/" + apiKey + "/conditions/lang:" + language + "/q/" + country + "/" + city + ".json");
}

// wunderground change the API URL scheme:
// http://api.wunderground.com/api/<API-KEY>/conditions/lang:de/q/zmw:00000.215.10348.json
void WundergroundConditions::updateConditions(WGConditions *conditions, String apiKey, String language, String zmwCode) {
  doUpdate(conditions, "http://api.wunderground.com/api/" + apiKey + "/conditions/lang:" + language + "/q/zmw:" + zmwCode + ".json");
}

void WundergroundConditions::updateConditionsPWS(WGConditions *conditions, String apiKey, String language, String pws) {
  doUpdate(conditions, "http://api.wunderground.com/api/" + apiKey + "/conditions/lang:" + language + "/q/pws:" + pws + ".json");
}

void WundergroundConditions::doUpdate(WGConditions *conditions, String url) {
  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();

  this->conditions = conditions;
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

    while(client->available() || client->connected()) {
      while((size = client->available()) > 0) {
		if ((millis() - lost_do) > lostTest) {
			Serial.println ("lost in client with a timeout");
			client->stop();
			ESP.restart();
	    }
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
  this->conditions = nullptr;
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
#ifdef NIGHTICONS
  if (currentKey == "icon_url") { //"icon_url":"http://icons.wxug.com/i/c/k/nt_clear.gif"
    int b1 = value.lastIndexOf('/') + 1;
    int b2 = value.lastIndexOf('.');
    conditions->weatherIcon = value.substring(b1, b2); //clear the begining and get from nt_ ... for night
  }
#else
  if (currentKey == "icon") {
    conditions->weatherIcon = value;
  }
#endif
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
