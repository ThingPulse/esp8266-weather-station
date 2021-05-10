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
#include "AerisForecasts.h"

AerisForecasts::AerisForecasts() {

}

void AerisForecasts::updateForecasts(AerisForecastData *forecasts, String clientId, String clientSecret, String location, uint8_t maxForecasts) {
  doUpdate(forecasts, "/forecasts/closest?p=" + location + "&client_id=" + clientId + "&client_secret=" + clientSecret, maxForecasts);
}

void AerisForecasts::doUpdate(AerisForecastData *forecasts, String path, uint8_t maxForecasts) {
  this->maxForecasts = maxForecasts;
  this->currentForecast = 0;
  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();

  this->forecasts = forecasts;
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
  this->forecasts = nullptr;
}

void AerisForecasts::whitespace(char c) {
  Serial.println("whitespace");
}

void AerisForecasts::startDocument() {
  Serial.println("start document");
}

void AerisForecasts::key(String key) {
  currentKey = String(key);
}

void AerisForecasts::value(String value) {
  // Serial.printf("Key: %s, value: %s\n", currentKey.c_str(), value.c_str());
  if (currentForecast >= maxForecasts) {
    //Serial.println("Skipping forecast. MaxForecasts reached");
    return;
  }
  // uint64_t timestamp; //  1526706000
  if (currentKey == "timestamp") {
    this->forecasts[currentForecast].timestamp = value.toInt();
  }
  // String validTime; // "2018-05-19T07:00:00+02:00"
  if (currentKey == "validTime") {
    this->forecasts[currentForecast].validTime = value;
  }
  // String dateTimeISO; //"2018-05-19T07:00:00+02:00"
  if (currentKey == "dateTimeISO") {
    this->forecasts[currentForecast].dateTimeISO = value;
  }
  // sint16_t maxTempC; //20
  if (currentKey == "maxTempC") {
    this->forecasts[currentForecast].maxTempC = value.toInt();
  }
  // sint16_t maxTempF; //69
  if (currentKey == "maxTempF") {
    this->forecasts[currentForecast].maxTempF = value.toInt();
  }
  // sint16_t minTempC; //14
  if (currentKey == "minTempC") {
    this->forecasts[currentForecast].minTempC = value.toInt();
  }
  // sint16_t minTempF; // 56
  if (currentKey == "minTempF") {
    this->forecasts[currentForecast].minTempF = value.toInt();
  }
  // sint16_t avgTempC; // 17
  if (currentKey == "avgTempC") {
    this->forecasts[currentForecast].avgTempC = value.toInt();
  }
  // sint16_t avgTempF; // 62
  if (currentKey == "avgTempF") {
    this->forecasts[currentForecast].avgTempF = value.toInt();
  }
  // sint16_t tempC; // null
  if (currentKey == "tempC") {
    this->forecasts[currentForecast].tempC = value.toInt();
  }
  // sint16_t tempF; // null
  if (currentKey == "tempF") {
    this->forecasts[currentForecast].tempF = value.toInt();
  }
  // sint16_t pop; // 20
  if (currentKey == "pop") {
    this->forecasts[currentForecast].pop = value.toInt();
  }
  // float precipMM; // 3.53
  if (currentKey == "precipMM") {
    this->forecasts[currentForecast].precipMM = value.toFloat();
  }
  // float precipIN; // 0.14
  if (currentKey == "precipIN") {
    this->forecasts[currentForecast].precipIN = value.toFloat();
  }
  // float iceaccum; // null
  if (currentKey == "iceaccum") {
    this->forecasts[currentForecast].iceaccum = value.toFloat();
  }
  // float iceaccumMM; // null
  if (currentKey == "iceaccumMM") {
    this->forecasts[currentForecast].iceaccumMM = value.toFloat();
  }
  // float iceaccumIN; // null
  if (currentKey == "iceaccumIN") {
    this->forecasts[currentForecast].iceaccumIN = value.toFloat();
  }
  // uint8_t maxHumidity; // 82
  if (currentKey == "maxHumidity") {
    this->forecasts[currentForecast].maxHumidity = value.toInt();
  }
  // uint8_t minHumidity; // 53
  if (currentKey == "minHumidity") {
    this->forecasts[currentForecast].minHumidity = value.toInt();
  }
  // uint8_t humidity; // 68
  if (currentKey == "humidity") {
    this->forecasts[currentForecast].humidity = value.toInt();
  }
  // uint8_t uvi; // 6
  if (currentKey == "uvi") {
    this->forecasts[currentForecast].uvi = value.toInt();
  }
  // uint16_t pressureMB; // 1018
  if (currentKey == "pressureMB") {
    this->forecasts[currentForecast].pressureMB = value.toInt();
  }
  // float pressureIN; // 30.06
  if (currentKey == "pressureIN") {
    this->forecasts[currentForecast].pressureIN = value.toFloat();
  }
  // uint8_t sky; // 99
  if (currentKey == "sky") {
    this->forecasts[currentForecast].sky = value.toInt();
  }
  // uint16_t snowCM; // 0
  if (currentKey == "snowCM") {
    this->forecasts[currentForecast].snowCM = value.toInt();
  }
  // uint16_t snowIN; // 0
  if (currentKey == "snowIN") {
    this->forecasts[currentForecast].snowIN = value.toInt();
  }
  // sint16_t feelslikeC; // 14
  if (currentKey == "feelslikeC") {
    this->forecasts[currentForecast].feelslikeC = value.toInt();
  }
  // sint16_t feelslikeF; // 56
  if (currentKey == "feelslikeF") {
    this->forecasts[currentForecast].feelslikeF = value.toInt();
  }
  // sint16_t minFeelslikeC; // 14
  if (currentKey == "minFeelslikeC") {
    this->forecasts[currentForecast].minFeelslikeC = value.toInt();
  }
  // sint16_t minFeelslikeF; // 56
  if (currentKey == "minFeelslikeF") {
    this->forecasts[currentForecast].minFeelslikeF = value.toInt();
  }
  // sint16_t maxFeelslikeC; // 20
  if (currentKey == "maxFeelslikeC") {
    this->forecasts[currentForecast].maxFeelslikeC = value.toInt();
  }
  // sint16_t maxFeelslikeF; // 69
  if (currentKey == "maxFeelslikeF") {
    this->forecasts[currentForecast].maxFeelslikeF = value.toInt();
  }
  // sint16_t avgFeelslikeC; // 17
  if (currentKey == "avgFeelslikeC") {
    this->forecasts[currentForecast].avgFeelslikeC = value.toInt();
  }
  // sint16_t avgFeelslikeF; // 63
  if (currentKey == "avgFeelslikeF") {
    this->forecasts[currentForecast].avgFeelslikeF = value.toInt();
  }
  // sint16_t dewpointC; // 11
  if (currentKey == "dewpointC") {
    this->forecasts[currentForecast].dewpointC = value.toInt();
  }
  // sint16_t dewpointF; // 51
  if (currentKey == "dewpointF") {
    this->forecasts[currentForecast].dewpointF = value.toInt();
  }
  // sint16_t maxDewpointC; // 13
  if (currentKey == "maxDewpointC") {
    this->forecasts[currentForecast].maxDewpointC = value.toInt();
  }
  // sint16_t maxDewpointF; // 55
  if (currentKey == "maxDewpointF") {
    this->forecasts[currentForecast].maxDewpointF = value.toInt();
  }
  // sint16_t minDewpointC; // 10
  if (currentKey == "minDewpointC") {
    this->forecasts[currentForecast].minDewpointC = value.toInt();
  }
  // sint16_t minDewpointF; // 51
  if (currentKey == "minDewpointF") {
    this->forecasts[currentForecast].minDewpointF = value.toInt();
  }
  // sint16_t avgDewpointC; // 11
  if (currentKey == "avgDewpointC") {
    this->forecasts[currentForecast].avgDewpointC = value.toInt();
  }
  // sint16_t avgDewpointF; // 52
  if (currentKey == "avgDewpointF") {
    this->forecasts[currentForecast].avgDewpointF = value.toInt();
  }
  // uint16_t windDirDEG; // 2
  if (currentKey == "windDirDEG") {
    this->forecasts[currentForecast].windDirDEG = value.toInt();
  }
  // String windDir; // "N"
  if (currentKey == "windDir") {
    this->forecasts[currentForecast].windDir = value;
  }
  // uint16_t windDirMaxDEG; // 40
  if (currentKey == "windDirMaxDEG") {
    this->forecasts[currentForecast].windDirMaxDEG = value.toInt();
  }
  // String windDirMax; // "NE"
  if (currentKey == "windDirMax") {
    this->forecasts[currentForecast].windDirMax = value;
  }
  // sint16_t windDirMinDEG; // 39
  if (currentKey == "windDirMinDEG") {
    this->forecasts[currentForecast].windDirMinDEG = value.toInt();
  }
  // String windDirMin; // "NE"
  if (currentKey == "windDirMin") {
    this->forecasts[currentForecast].windDirMin = value;
  }
  // uint16_t windGustKTS; // 6
  if (currentKey == "windGustKTS") {
    this->forecasts[currentForecast].windGustKTS = value.toInt();
  }
  // uint16_t windGustKPH; // 11
  if (currentKey == "windGustKPH") {
    this->forecasts[currentForecast].windGustKPH = value.toInt();
  }
  // uint16_t windGustMPH; // 7
  if (currentKey == "windGustMPH") {
    this->forecasts[currentForecast].windGustMPH = value.toInt();
  }
  // uint16_t windSpeedKTS; // 4
  if (currentKey == "windSpeedKTS") {
    this->forecasts[currentForecast].windSpeedKTS = value.toInt();
  }
  // uint16_t windSpeedKPH; // 7
  if (currentKey == "windSpeedKPH") {
    this->forecasts[currentForecast].windSpeedKPH = value.toInt();
  }
  // uint16_t windSpeedMPH; // 5
  if (currentKey == "windSpeedMPH") {
    this->forecasts[currentForecast].windSpeedMPH = value.toInt();
  }
  // uint16_t windSpeedMaxKTS; // 6
  if (currentKey == "windSpeedMaxKTS") {
    this->forecasts[currentForecast].windSpeedMaxKTS = value.toInt();
  }
  // uint16_t windSpeedMaxKPH; // 11
  if (currentKey == "windSpeedMaxKPH") {
    this->forecasts[currentForecast].windSpeedMaxKPH = value.toInt();
  }
  // uint16_t windSpeedMaxMPH; // 7
  if (currentKey == "windSpeedMaxMPH") {
    this->forecasts[currentForecast].windSpeedMaxMPH = value.toInt();
  }
  // uint16_t windSpeedMinKTS; // 1
  if (currentKey == "windSpeedMinKTS") {
    this->forecasts[currentForecast].windSpeedMinKTS = value.toInt();
  }
  // uint16_t windSpeedMinKPH; // 2
  if (currentKey == "windSpeedMinKPH") {
    this->forecasts[currentForecast].windSpeedMinKPH = value.toInt();
  }
  // uint16_t windSpeedMinMPH; // 1
  if (currentKey == "windSpeedMinMPH") {
    this->forecasts[currentForecast].windSpeedMinMPH = value.toInt();
  }
  // uint16_t windDir80mDEG; // 5
  if (currentKey == "windDir80mDEG") {
    this->forecasts[currentForecast].windDir80mDEG = value.toInt();
  }
  // String windDir80m; // "N"
  if (currentKey == "windDir80m") {
    this->forecasts[currentForecast].windDir80m = value;
  }
  // uint16_t windDirMax80mDEG; // 40
  if (currentKey == "windDirMax80mDEG") {
    this->forecasts[currentForecast].windDirMax80mDEG = value.toInt();
  }
  // String windDirMax80m; // "NE"
  if (currentKey == "windDirMax80m") {
    this->forecasts[currentForecast].windDirMax80m = value;
  }
  // uint16_t windDirMin80mDEG; // 39
  if (currentKey == "windDirMin80mDEG") {
    this->forecasts[currentForecast].windDirMin80mDEG = value.toInt();
  }
  // String windDirMin80m; // "NE"
  if (currentKey == "windDirMin80m") {
    this->forecasts[currentForecast].windDirMin80m = value;
  }
  // uint16_t windGust80mKTS; // 9
  if (currentKey == "windGust80mKTS") {
    this->forecasts[currentForecast].windGust80mKTS = value.toInt();
  }
  // uint16_t windGust80mKPH; // 17
  if (currentKey == "windGust80mKPH") {
    this->forecasts[currentForecast].windGust80mKPH = value.toInt();
  }
  // uint16_t windGust80mMPH; // 11
  if (currentKey == "windGust80mMPH") {
    this->forecasts[currentForecast].windGust80mMPH = value.toInt();
  }
  // uint16_t windSpeed80mKTS; // 6
  if (currentKey == "windSpeed80mKTS") {
    this->forecasts[currentForecast].windSpeed80mKTS = value.toInt();
  }
  // uint16_t windSpeed80mKPH; // 11
  if (currentKey == "windSpeed80mKPH") {
    this->forecasts[currentForecast].windSpeed80mKPH = value.toInt();
  }
  // uint16_t windSpeed80mMPH; // 7
  if (currentKey == "windSpeed80mMPH") {
    this->forecasts[currentForecast].windSpeed80mMPH = value.toInt();
  }
  // uint16_t windSpeedMax80mKTS; // 9
  if (currentKey == "windSpeedMax80mKTS") {
    this->forecasts[currentForecast].windSpeedMax80mKTS = value.toInt();
  }
  // uint16_t windSpeedMax80mKPH; // 17
  if (currentKey == "windSpeedMax80mKPH") {
    this->forecasts[currentForecast].windSpeedMax80mKPH = value.toInt();
  }
  // uint16_t windSpeedMax80mMPH; // 11
  if (currentKey == "windSpeedMax80mMPH") {
    this->forecasts[currentForecast].windSpeedMax80mMPH = value.toInt();
  }
  // uint16_t windSpeedMin80mKTS; // 4
  if (currentKey == "windSpeedMin80mKTS") {
    this->forecasts[currentForecast].windSpeedMin80mKTS = value.toInt();
  }
  // uint16_t windSpeedMin80mKPH; // 7
  if (currentKey == "windSpeedMin80mKPH") {
    this->forecasts[currentForecast].windSpeedMin80mKPH = value.toInt();
  }
  // uint16_t windSpeedMin80mMPH; // 4
  if (currentKey == "windSpeedMin80mMPH") {
    this->forecasts[currentForecast].windSpeedMin80mMPH = value.toInt();
  }
  // String weather; // "Cloudy with Drizzle"
  if (currentKey == "weather") {
    this->forecasts[currentForecast].weather = value;
  }
  // String weatherPrimary; // "Drizzle"
  if (currentKey == "weatherPrimary") {
    this->forecasts[currentForecast].weatherPrimary = value;
  }
  // String weatherPrimaryCoded; // "IS:VL:RW"
  if (currentKey == "weatherPrimaryCoded") {
    this->forecasts[currentForecast].weatherPrimaryCoded = value;
  }
  // String cloudsCoded; // "OV"
  if (currentKey == "cloudsCoded") {
    this->forecasts[currentForecast].cloudsCoded = value;
  }
  // String icon; // "drizzle.png"
  // String iconMeteoCon; // Q
  if (currentKey == "icon") {
    this->forecasts[currentForecast].icon = value;
    this->forecasts[currentForecast].iconMeteoCon = getMeteoconIcon(value);
  }
  // boolean isDay; // true
  if (currentKey == "isDay") {
    this->forecasts[currentForecast].isDay = currentKey == "true" ? true : false;
  }
  // uint64_t sunrise; // 1526701425
  if (currentKey == "sunrise") {
    this->forecasts[currentForecast].sunrise = value.toInt();
  }
  // String sunriseISO; // "2018-05-19T05:43:45+02:00"
  if (currentKey == "sunriseISO") {
    this->forecasts[currentForecast].sunriseISO = value;
  }
  // uint64_t sunset; // 1526756450
  if (currentKey == "sunset") {
    this->forecasts[currentForecast].sunset = value.toInt();
  }
  // String sunsetISO; // "2018-05-19T21:00:50+02:00"
  if (currentKey == "sunsetISO") {
    this->forecasts[currentForecast].sunsetISO = value;
    // last element. increase currentForecast
    currentForecast++;
  }
}

void AerisForecasts::endArray() {

}


void AerisForecasts::startObject() {
  currentParent = currentKey;
}

void AerisForecasts::endObject() {
  currentParent = "";
}

void AerisForecasts::endDocument() {

}

void AerisForecasts::startArray() {

}


String AerisForecasts::getMeteoconIcon(String iconText) {
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
