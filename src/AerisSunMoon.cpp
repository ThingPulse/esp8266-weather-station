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
#include <WiFiClient.h>
#include "AerisSunMoon.h"

AerisSunMoon::AerisSunMoon() {

}

void AerisSunMoon::updateSunMoon(AerisSunMoonData *sunMoonData, String clientId, String clientSecret, String location) {
  doUpdate(sunMoonData, "/sunmoon/" + location + "?client_id=" + clientId + "&client_secret=" + clientSecret);
}

void AerisSunMoon::doUpdate(AerisSunMoonData *sunMoonData, String path) {
  this->sunMoonData = sunMoonData;

  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();

  JsonStreamingParser parser;
  parser.setListener(this);
  Serial.printf("[HTTP] Requesting resource at http://%s:%u%s\n", host.c_str(), port, path.c_str());

  WiFiClient client;
  if(client.connect(host.c_str(), port)) {
    bool isBody = false;
    char c;
    Serial.println("[HTTP] connected, now GETting data");
    client.print("GET " + path + " HTTP/1.1\r\n"
                 "Host: " + host + "\r\n"
                 "Connection: close\r\n\r\n");

    while (client.connected() || client.available()) {
      if (client.available()) {
        if ((millis() - lost_do) > lostTest) {
          Serial.println("[HTTP] lost in client with a timeout");
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
    Serial.println("[HTTP] failed to connect to host");
  }
  this->sunMoonData = nullptr;
}

void AerisSunMoon::whitespace(char c) {
  Serial.println("whitespace");
}

void AerisSunMoon::startDocument() {
  Serial.println("start document");
}

void AerisSunMoon::key(String key) {
  currentKey = String(key);
}

void AerisSunMoon::value(String value) {
  // uint64_t sunRise; // "rise":1493291184,
  if (currentParent == "sun") {
    if (currentKey == "rise") {
      this->sunMoonData->sunRise = value.toInt();
    }
    // String sunRiseISO; // "riseISO":"2017-04-27T06:06:24-05:00",
    if (currentKey == "riseISO") {
      this->sunMoonData->sunRiseISO = value;
    }
    // uint64_t sunSet; // "set":1493342079,
    if (currentKey == "set") {
      this->sunMoonData->sunSet = value.toInt();
    }
    // String sunSetISO; //"setISO":"2017-04-27T20:14:39-05:00",
    if (currentKey == "setISO") {
      this->sunMoonData->sunSetISO = value;
    }
    // uint64_t sunTransit; // "transit":1493316631,
    if (currentKey == "transit") {
      this->sunMoonData->sunTransit = value.toInt();
    }
    // String sunTransitISO; // "transitISO":"2017-04-27T13:10:31-05:00",
    if (currentKey == "transitISO") {
      this->sunMoonData->sunTransitISO = value;
    }
    // boolean midnightSun; // "midnightSun":false,
    if (currentKey == "midnightSun") {
      this->sunMoonData->midnightSun = (value == "true" ? true : false);
    }
    // boolean polarNight; // "polarNight":false,
    if (currentKey == "polarNight") {
      this->sunMoonData->polarNight = (value == "true" ? true : false);
    }
  }
  if (currentParent == "moon") {
    // uint64_t moonRise; //"rise":1493295480,
    if (currentKey == "rise") {
      this->sunMoonData->moonRise = value.toInt();
    }
    // String moonRiseISO; // "riseISO":"2017-04-27T07:18:00-05:00",
    if (currentKey == "riseISO") {
      this->sunMoonData->moonRiseISO = value;
    }
    // uint64_t moonSet; // "set":1493347800,
    if (currentKey == "set") {
      this->sunMoonData->moonSet = value.toInt();
    }
    // String moonSetISO; // "setISO":"2017-04-27T21:50:00-05:00",
    if (currentKey == "setISO") {
      this->sunMoonData->moonSetISO = value;
    }
    // uint64_t moonTransit; // "transit":1493321340,
    if (currentKey == "transit") {
      this->sunMoonData->moonTransit = value.toInt();
    }
    // String moonTransitISO; // "transitISO":"2017-04-27T14:29:00-05:00",
    if (currentKey == "transitISO") {
      this->sunMoonData->moonTransitISO = value;
    }
    // uint64_t moonUnderfoot; // "underfoot":1493276400,
    if (currentKey == "underfoot") {
      this->sunMoonData->moonUnderfoot = value.toInt();
    }
    // String moonUnderfootISO; // "underfootISO":"2017-04-27T02:00:00-05:00",
    if (currentKey == "underfootISO") {
      this->sunMoonData->moonUnderfootISO = value;
    }
  }
  if (currentParent == "phase") {
    // float moonPhase; // "phase":0.0516,
    if (currentKey == "phase") {
      this->sunMoonData->moonPhase = value.toFloat();
    }
    // String moonPhaseName; // "name":"waxing crescent",
    if (currentKey == "name") {
      this->sunMoonData->moonPhaseName = value;
    }
    // uint8_t moonIllum; // "illum":3,
    if (currentKey == "illum") {
      this->sunMoonData->moonIllum = value.toInt();
    }
    // float moonAge; // "age":1.52,
    if (currentKey == "age") {
      this->sunMoonData->moonAge = value.toFloat();
    }
    // float moonAngle; // "angle":0.55
    if (currentKey == "angle") {
      this->sunMoonData->moonAngle = value.toFloat();
    }
  }
}

void AerisSunMoon::endArray() {

}


void AerisSunMoon::startObject() {
  Serial.println("Starting new object: " + currentKey);
  currentParent = currentKey;
}

void AerisSunMoon::endObject() {
  currentParent = "";
}

void AerisSunMoon::endDocument() {

}

void AerisSunMoon::startArray() {

}
