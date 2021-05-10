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
#include "AerisObservations.h"

AerisObservations::AerisObservations() {

}

void AerisObservations::updateObservations(AerisObservationsData *observations, String clientId, String clientSecret, String location) {
  doUpdate(observations, "/observations/closest?p=" + location + "&client_id=" + clientId + "&client_secret=" + clientSecret);
}

void AerisObservations::doUpdate(AerisObservationsData *observations, String path) {
  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();

  this->observations = observations;
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
  this->observations = nullptr;
}

void AerisObservations::whitespace(char c) {
  Serial.println("whitespace");
}

void AerisObservations::startDocument() {
  Serial.println("start document");
}

void AerisObservations::key(String key) {
  currentKey = String(key);
}

void AerisObservations::value(String value) {
  // Serial.printf("Key: %s, value: %s\n", currentKey.c_str(), value.c_str());
  //uint64_t timestamp;
  if (currentKey == "timestamp") {
    this->observations->timestamp = value.toInt();
  }
  // String dateTimeISO;
  if (currentKey == "dateTimeISO") {
    this->observations->dateTimeISO = value;
  }
  // sint16_t tempC;
  if (currentKey == "tempC") {
    this->observations->tempC = value.toInt();
  }
  // sint16_t tempF;
  if (currentKey == "tempF") {
    this->observations->tempF = value.toInt();
  }
  // sint16_t dewpointC;
  if (currentKey == "dewpointC") {
    this->observations->dewpointC = value.toInt();
  }
  // sint16_t dewpointF;
  if (currentKey == "dewpointF") {
    this->observations->dewpointF = value.toInt();
  }
  // uint8_t humidity;
  if (currentKey == "humidity") {
    this->observations->humidity = value.toInt();
  }
  // uint16_t pressureMB;
  if (currentKey == "pressureMB") {
    this->observations->pressureMB = value.toInt();
  }
  // float pressureIN;
  if (currentKey == "pressureIN") {
    this->observations->pressureIN = value.toFloat();
  }
  // uint16_t spressureMB;
  if (currentKey == "spressureMB") {
    this->observations->spressureMB = value.toInt();
  }
  // float spressureIN;
  if (currentKey == "spressureIN") {
    this->observations->spressureIN = value.toFloat();
  }
  // uint16_t altimeterMB;
  if (currentKey == "altimeterMB") {
    this->observations->altimeterMB = value.toInt();
  }
  // float altimeterIN;
  if (currentKey == "altimeterIN") {
    this->observations->altimeterIN = value.toFloat();
  }
  // uint16_t windSpeedKTS;
  if (currentKey == "windSpeedKTS") {
    this->observations->windSpeedKTS = value.toInt();
  }
  // uint16_t windSpeedKPH;
  if (currentKey == "windSpeedKPH") {
    this->observations->windSpeedKPH = value.toInt();
  }
  // uint16_t windSpeedMPH;
  if (currentKey == "windSpeedMPH") {
    this->observations->windSpeedMPH = value.toInt();
  }
  // uint16_t windDirDEG;
  if (currentKey == "windDirDEG") {
    this->observations->windDirDEG = value.toInt();
  }
  // String windDir;
  if (currentKey == "windDir") {
    this->observations->windDir = value;
  }
  // uint16_t windGustKTS;
  if (currentKey == "windGustKTS") {
    this->observations->windGustKTS = value.toInt();
  }
  // uint16_t windGustKPH;
  if (currentKey == "windGustKPH") {
    this->observations->windGustKPH = value.toInt();
  }
  // uint16_t windGustMPH;
  if (currentKey == "windGustMPH") {
    this->observations->windGustMPH = value.toInt();
  }
  // String flightRule;
  if (currentKey == "flightRule") {
    this->observations->flightRule = value;
  }
  // float visibilityKM;
  if (currentKey == "visibilityKM") {
    this->observations->visibilityKM = value.toFloat();
  }
  // float visibilityMI;
  if (currentKey == "visibilityMI") {
    this->observations->visibilityMI = value.toFloat();
  }
  // String weather;
  if (currentKey == "weather") {
    this->observations->weather = value;
  }
  // String weatherShort;
  if (currentKey == "weatherShort") {
    this->observations->weatherShort = value;
  }
  // String weatherCoded;
  if (currentKey == "weatherCoded") {
    this->observations->weatherCoded = value;
  }
  // String weatherPrimary;
  if (currentKey == "weatherPrimary") {
    this->observations->weatherPrimary = value;
  }
  // String weatherPrimaryCoded;
  if (currentKey == "weatherPrimaryCoded") {
    this->observations->weatherPrimaryCoded = value;
  }
  // String cloudsCoded;
  if (currentKey == "cloudsCoded") {
    this->observations->cloudsCoded = value;
  }
  // String icon;
  // String iconMeteoCon;
  if (currentKey == "icon") {
    this->observations->icon = value;
    this->observations->iconMeteoCon = getMeteoconIcon(value);
  }
  // sint16_t heatindexC;
  if (currentKey == "heatindexC") {
    this->observations->heatindexC = value.toInt();
  }
  // sint16_t heatindexF;
  if (currentKey == "heatindexF") {
    this->observations->heatindexF = value.toInt();
  }
  // sint16_t windchillC;
  if (currentKey == "windchillC") {
    this->observations->windchillC = value.toInt();
  }
  // sint16_t windchillF;
  if (currentKey == "windchillF") {
    this->observations->windchillF = value.toInt();
  }
  // sint16_t feelslikeC;
  if (currentKey == "feelslikeC") {
    this->observations->feelslikeC = value.toInt();
  }
  // sint16_t feelslikeF;
  if (currentKey == "feelslikeF") {
    this->observations->feelslikeF = value.toInt();
  }
  // boolean isDay;
  if (currentKey == "isDay") {
    this->observations->isDay = value == "true" ? true : false;
  }
  // uint64_t sunrise;
  if (currentKey == "sunrise") {
    this->observations->sunrise = value.toInt();
  }
  // String sunriseISO;
  if (currentKey == "sunriseISO") {
    this->observations->sunriseISO = value;
  }
  // uint64_t sunset;
  if (currentKey == "sunset") {
    this->observations->sunset = value.toInt();
  }
  // String sunsetISO;
  if (currentKey == "sunsetISO") {
    this->observations->sunsetISO = value;
  }
  // uint16_t snowDepthCM;
  if (currentKey == "snowDepthCM") {
    this->observations->snowDepthCM = value.toInt();
  }
  // uint16_t snowDepthIN;
  if (currentKey == "snowDepthIN") {
    this->observations->snowDepthIN = value.toInt();
  }
  // uint16_t precipMM;
  if (currentKey == "precipMM") {
    this->observations->precipMM = value.toInt();
  }
  // uint16_t precipIN;
  if (currentKey == "precipIN") {
    this->observations->precipIN = value.toInt();
  }
  // uint16_t solradWM2;
  if (currentKey == "solradWM2") {
    this->observations->solradWM2 = value.toInt();
  }
  // String solradMethod;
  if (currentKey == "solradMethod") {
    this->observations->solradMethod = value;
  }
  // uint16_t light;
  if (currentKey == "light") {
    this->observations->light = value.toInt();
  }
  // uint16_t sky;
  if (currentKey == "sky") {
    this->observations->sky = value.toInt();
  }
}

void AerisObservations::endArray() {

}


void AerisObservations::startObject() {
  currentParent = currentKey;
}

void AerisObservations::endObject() {
  currentParent = "";
}

void AerisObservations::endDocument() {

}

void AerisObservations::startArray() {

}


String AerisObservations::getMeteoconIcon(String iconText) {
  if (iconText == "am_pcloudyr.png") return "Q";
  if (iconText == "am_showers.png") return "R";
  if (iconText == "am_showshowers.png") return "W";
  if (iconText == "am_tstorm.png") return "P";
  if (iconText == "blizzard.png") return "W";
  if (iconText == "blizzardn.png") return "#";
  if (iconText == "blowingsnow.png") return "W";
  if (iconText == "blowingsnown.png") return "#";
  if (iconText == "chancetstorm.png") return "P";
  if (iconText == "chancetstormn.png") return "6";
  if (iconText == "clear.png") return "B";
  if (iconText == "clearn.png") return "w";
  if (iconText == "clearw.png") return "B";
  if (iconText == "clearwn.png") return "2";
  if (iconText == "cloudy.png") return "N";
  if (iconText == "cloudyn.png") return "5";
  if (iconText == "cloudyw.png") return "S";
  if (iconText == "cloudywn.png") return "!";
  if (iconText == "drizzle.png") return "Q";
  if (iconText == "drizzlef.png") return "L";
  if (iconText == "drizzlen.png") return "7";
  if (iconText == "dust.png") return "M";
  if (iconText == "fair.png") return "H";
  if (iconText == "fairn.png") return "4";
  if (iconText == "fairw.png") return "H";
  if (iconText == "fairwn.png") return "4";
  if (iconText == "fdrizzle.png") return "W";
  if (iconText == "fdrizzlen.png") return "#";
  if (iconText == "flurries.png") return "V";
  if (iconText == "flurriesn.png") return "#";
  if (iconText == "flurriesw.png") return "V";
  if (iconText == "flurrieswn.png") return "#";
  if (iconText == "fog.png") return "M";
  if (iconText == "fogn.png") return "M";
  if (iconText == "freezingrain.png") return "X";
  if (iconText == "freezingrainn.png") return "$";
  if (iconText == "hazy.png") return "J";
  if (iconText == "hazyn.png") return "K";
  if (iconText == "mcloudy.png") return "H";
  if (iconText == "mcloudyn.png") return "4";
  if (iconText == "mcloudyr.png") return "Q";
  if (iconText == "mcloudyrn.png") return "7";
  if (iconText == "mcloudyrw.png") return "S";
  if (iconText == "mcloudyrwn.png") return "!";
  if (iconText == "mcloudys.png") return "V";
  if (iconText == "mcloudysfn.png") return "#";
  if (iconText == "mcloudysfw.png") return "S";
  if (iconText == "mcloudysfwn.png") return "9";
  if (iconText == "mcloudysn.png") return "9";
  if (iconText == "mcloudysw.png") return "W";
  if (iconText == "mcloudyswn.png") return "#";
  if (iconText == "mcloudyt.png") return "P";
  if (iconText == "mcloudytn.png") return "6";
  if (iconText == "mcloudytw.png") return "P";
  if (iconText == "mcloudytwn.png") return "6";
  if (iconText == "mcloudyw.png") return "S";
  if (iconText == "mcloudywn.png") return "9";
  if (iconText == "na.png") return ")";
  if (iconText == "pcloudy.png") return "H";
  if (iconText == "pcloudyn.png") return "4";
  if (iconText == "pcloudyr.png") return "Q";
  if (iconText == "pcloudyrn.png") return "7";
  if (iconText == "pcloudyrw.png") return "T";
  if (iconText == "pcloudyrwn.png") return "!";
  if (iconText == "pcloudys.png") return "W";
  if (iconText == "pcloudysf.png") return "V";
  if (iconText == "pcloudysfn.png") return "\"";
  if (iconText == "pcloudysfw.png") return "U";
  if (iconText == "pcloudysfwn.png") return "\"";
  if (iconText == "pcloudysn.png") return "\"";
  if (iconText == "pcloudysw.png") return "U";
  if (iconText == "pcloudyswn.png") return "\"";
  if (iconText == "pcloudyt.png") return "P";
  if (iconText == "pcloudytn.png") return "6";
  if (iconText == "pcloudytw.png") return "P";
  if (iconText == "pcloudytwn.png") return "6";
  if (iconText == "pcloudyw.png") return "S";
  if (iconText == "pcloudywn.png") return "!";
  if (iconText == "pm_pcloudy.png") return "H";
  if (iconText == "pm_pcloudyr.png") return "Q";
  if (iconText == "pm_showers.png") return "R";
  if (iconText == "pm_snowshowers.png") return "W";
  if (iconText == "pm_tstorm.png") return "P";
  if (iconText == "rain.png") return "R";
  if (iconText == "rainandsnow.png") return "W";
  if (iconText == "rainandsnown.png") return "#";
  if (iconText == "rainn.png") return "8";
  if (iconText == "raintosnow.png") return "W";
  if (iconText == "raintosnown.png") return "#";
  if (iconText == "rainw.png") return "S";
  if (iconText == "showers.png") return "W";
  if (iconText == "showersn.png") return "#";
  if (iconText == "showersw.png") return "T";
  if (iconText == "sleet.png") return "X";
  if (iconText == "sleetn.png") return "$";
  if (iconText == "sleetsnow.png") return "X";
  if (iconText == "smoke.png") return "M";
  if (iconText == "smoken.png") return "M";
  if (iconText == "snow.png") return "W";
  if (iconText == "snown.png") return "#";
  if (iconText == "snowshowers.png") return "W";
  if (iconText == "snowshowersn.png") return "#";
  if (iconText == "snowshowersw.png") return "W";
  if (iconText == "snowshowerswn.png") return "#";
  if (iconText == "snowtorain.png") return "W";
  if (iconText == "snowtorainn.png") return "#";
  if (iconText == "snoww.png") return "W";
  if (iconText == "snowwn.png") return "#";
  if (iconText == "sunny.png") return "B";
  if (iconText == "sunnyn.png") return "2";
  if (iconText == "sunnyw.png") return "F";
  if (iconText == "sunnywn.png") return "F";
  if (iconText == "tstorm.png") return "0";
  if (iconText == "tstormn.png") return "&";
  if (iconText == "tstormsw.png") return "0";
  if (iconText == "tstormswn.png") return "&";
  if (iconText == "tstormw.png") return "0";
  if (iconText == "tstormwn.png") return "&";
  if (iconText == "wind.png") return "F";
  if (iconText == "wintrymix.png") return "W";
  if (iconText == "wintrymixn.png") return "#";
  return ")";

}
