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
#include "WundergroundAstronomy.h"


WundergroundAstronomy::WundergroundAstronomy(boolean _usePM) {
  usePM = _usePM;
}
void WundergroundAstronomy::setPM(boolean usePM) {
  this->usePM = usePM;
}
void WundergroundAstronomy::updateAstronomy(WGAstronomy *astronomy, String apiKey, String language, String country, String city) {
  doUpdate(astronomy, "http://api.wunderground.com/api/" + apiKey + "/astronomy/lang:" + language + "/q/" + country + "/" + city + ".json");
}

void WundergroundAstronomy::updateAstronomyPWS(WGAstronomy *astronomy, String apiKey, String language, String pws) {
  doUpdate(astronomy, "http://api.wunderground.com/api/" + apiKey + "/astronomy/lang:" + language + "/q/pws:" + pws + ".json");
}

void WundergroundAstronomy::doUpdate(WGAstronomy *astronomy, String url) {
  this->astronomy = astronomy;
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
  this->astronomy = nullptr;
}

void WundergroundAstronomy::whitespace(char c) {
  Serial.println("whitespace");
}

void WundergroundAstronomy::startDocument() {
  Serial.println("start document");
}

void WundergroundAstronomy::key(String key) {
  currentKey = String(key);
}

void WundergroundAstronomy::value(String value) {

  if (currentKey == "ageOfMoon") {
    astronomy->moonAge = value;
  }

  if (currentKey == "phaseofMoon") {
    astronomy->moonPhase = value;
  }

  if (currentKey == "percentIlluminated") {
    astronomy->moonPctIlum = value;
  }


  if (currentParent == "sunrise") {      // Has a Parent key and 2 sub-keys
    if (currentKey == "hour") {
      int tempHour = value.toInt();    // do this to concert to 12 hour time (make it a function!)
      if (usePM && tempHour > 12){
        tempHour -= 12;
        isPM = true;
      }
      else {
        isPM = false;
      }
      char tempHourBuff[3] = "";
      sprintf(tempHourBuff, "%02d", tempHour);
      astronomy->sunriseTime = String(tempHourBuff);
    }
    if (currentKey == "minute") {
      char tempMinBuff[4] = "";
      if (usePM) {
        sprintf(tempMinBuff, "%02d%s", value.toInt(), isPM?"pm":"am");
      } else {
        sprintf(tempMinBuff, "%02d", value.toInt());
      }
      astronomy->sunriseTime += ":" + String(tempMinBuff);

    }
    this->sunriseTime.trim();
  }


  if (currentParent == "sunset") {      // Has a Parent key and 2 sub-keys
    if (currentKey == "hour") {
      int tempHour = value.toInt();   // do this to concert to 12 hour time (make it a function!)
      if (usePM && tempHour > 12) {
        tempHour -= 12;
        isPM = true;
      } else {
        isPM = false;
      }
      char tempHourBuff[3] = "";
      sprintf(tempHourBuff, "%02d", tempHour);
      astronomy->sunsetTime = String(tempHourBuff);
    }
    if (currentKey == "minute") {
      char tempMinBuff[4] = "";
      if (usePM) {
        sprintf(tempMinBuff, "%02d%s", value.toInt(), isPM?"pm":"am");
      } else {
        sprintf(tempMinBuff, "%02d", value.toInt());
      }
      astronomy->sunsetTime += ":" + String(tempMinBuff);
    }
    this->sunsetTime.trim();
  }

  if (currentParent == "moonrise") {      // Has a Parent key and 2 sub-keys
    if (currentKey == "hour") {
    int tempHour = value.toInt();   // do this to concert to 12 hour time (make it a function!)
    if (usePM && tempHour > 12){
      tempHour -= 12;
      isPM = true;
    }
    else isPM = false;
    char tempHourBuff[3] = "";
    sprintf(tempHourBuff, "%02d", tempHour);
    astronomy->moonriseTime = String(tempHourBuff);
    }
    if (currentKey == "minute") {
      char tempMinBuff[4] = "";
      if (usePM) {
        sprintf(tempMinBuff, "%02d%s", value.toInt(), isPM?"pm":"am");
      } else {
        sprintf(tempMinBuff, "%02d", value.toInt());
      }
      astronomy->moonriseTime += ":" + String(tempMinBuff);
    }
    this->moonriseTime.trim();
  }

  if (currentParent == "moonset") {      // Has a Parent key and 2 sub-keys
    if (currentKey == "hour") {
      int tempHour = value.toInt();   // do this to concert to 12 hour time (make it a function!)
      if (usePM && tempHour > 12){
        tempHour -= 12;
        isPM = true;
      }
      else isPM = false;
      char tempHourBuff[3] = "";
      sprintf(tempHourBuff, "%02d", tempHour);
      astronomy->moonsetTime = String(tempHourBuff);
    }
    if (currentKey == "minute") {
      char tempMinBuff[4] = "";
      if (usePM) {
        sprintf(tempMinBuff, "%02d%s", value.toInt(), isPM?"pm":"am");
      } else {
        sprintf(tempMinBuff, "%02d", value.toInt());
      }
      astronomy->moonsetTime += ":" + String(tempMinBuff);
    }
    astronomy->moonsetTime.trim();
  }

}

void WundergroundAstronomy::endArray() {

}


void WundergroundAstronomy::startObject() {
  currentParent = currentKey;
}

void WundergroundAstronomy::endObject() {
  currentParent = "";
}

void WundergroundAstronomy::endDocument() {

}

void WundergroundAstronomy::startArray() {

}
