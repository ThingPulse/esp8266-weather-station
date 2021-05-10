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

#pragma once
#include <Arduino.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>

typedef struct AerisForecastData {
  uint64_t timestamp; //  1526706000
  String validTime; // "2018-05-19T07:00:00+02:00"
  String dateTimeISO; //"2018-05-19T07:00:00+02:00"
  int16_t maxTempC; //20
  int16_t maxTempF; //69
  int16_t minTempC; //14
  int16_t minTempF; // 56
  int16_t avgTempC; // 17
  int16_t avgTempF; // 62
  int16_t tempC; // null
  int16_t tempF; // null
  int16_t pop; // 20
  float precipMM; // 3.53
  float precipIN; // 0.14
  float iceaccum; // null
  float iceaccumMM; // null
  float iceaccumIN; // null
  uint8_t maxHumidity; // 82
  uint8_t minHumidity; // 53
  uint8_t humidity; // 68
  uint8_t uvi; // 6
  uint16_t pressureMB; // 1018
  float pressureIN; // 30.06
  uint8_t sky; // 99
  uint16_t snowCM; // 0
  uint16_t snowIN; // 0
  int16_t feelslikeC; // 14
  int16_t feelslikeF; // 56
  int16_t minFeelslikeC; // 14
  int16_t minFeelslikeF; // 56
  int16_t maxFeelslikeC; // 20
  int16_t maxFeelslikeF; // 69
  int16_t avgFeelslikeC; // 17
  int16_t avgFeelslikeF; // 63
  int16_t dewpointC; // 11
  int16_t dewpointF; // 51
  int16_t maxDewpointC; // 13
  int16_t maxDewpointF; // 55
  int16_t minDewpointC; // 10
  int16_t minDewpointF; // 51
  int16_t avgDewpointC; // 11
  int16_t avgDewpointF; // 52
  uint16_t windDirDEG; // 2
  String windDir; // "N"
  uint16_t windDirMaxDEG; // 40
  String windDirMax; // "NE"
  int16_t windDirMinDEG; // 39
  String windDirMin; // "NE"
  uint16_t windGustKTS; // 6
  uint16_t windGustKPH; // 11
  uint16_t windGustMPH; // 7
  uint16_t windSpeedKTS; // 4
  uint16_t windSpeedKPH; // 7
  uint16_t windSpeedMPH; // 5
  uint16_t windSpeedMaxKTS; // 6
  uint16_t windSpeedMaxKPH; // 11
  uint16_t windSpeedMaxMPH; // 7
  uint16_t windSpeedMinKTS; // 1
  uint16_t windSpeedMinKPH; // 2
  uint16_t windSpeedMinMPH; // 1
  uint16_t windDir80mDEG; // 5
  String windDir80m; // "N"
  uint16_t windDirMax80mDEG; // 40
  String windDirMax80m; // "NE"
  uint16_t windDirMin80mDEG; // 39
  String windDirMin80m; // "NE"
  uint16_t windGust80mKTS; // 9
  uint16_t windGust80mKPH; // 17
  uint16_t windGust80mMPH; // 11
  uint16_t windSpeed80mKTS; // 6
  uint16_t windSpeed80mKPH; // 11
  uint16_t windSpeed80mMPH; // 7
  uint16_t windSpeedMax80mKTS; // 9
  uint16_t windSpeedMax80mKPH; // 17
  uint16_t windSpeedMax80mMPH; // 11
  uint16_t windSpeedMin80mKTS; // 4
  uint16_t windSpeedMin80mKPH; // 7
  uint16_t windSpeedMin80mMPH; // 4
  String weather; // "Cloudy with Drizzle"
  String weatherPrimary; // "Drizzle"
  String weatherPrimaryCoded; // "IS:VL:RW"
  String cloudsCoded; // "OV"
  String icon; // "drizzle.png"
  String iconMeteoCon; // Q
  boolean isDay; // true
  uint64_t sunrise; // 1526701425
  String sunriseISO; // "2018-05-19T05:43:45+02:00"
  uint64_t sunset; // 1526756450
  String sunsetISO; // "2018-05-19T21:00:50+02:00"
} AerisForecastData;

class AerisForecasts: public JsonListener {
  private:
    const String host = "api.aerisapi.com";
    const uint8_t port = 80;
    boolean isMetric = true;
    String currentKey;
    String currentParent;
    AerisForecastData *forecasts;
    uint8_t maxForecasts;
    uint8_t currentForecast;


  void doUpdate(AerisForecastData *forecasts, String url, uint8_t maxForecasts);

  public:
    AerisForecasts();
    void updateForecasts(AerisForecastData *forecasts, String clientId, String clientKey, String location, uint8_t maxForecasts);

    String getMeteoconIcon(String icon);


    virtual void whitespace(char c);

    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();
};
