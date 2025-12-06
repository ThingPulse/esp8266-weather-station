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
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <time.h>

typedef struct VisualCrossingData {
  // "latitude": 47.37
  float latitude;
  // "longitude": 8.54,
  float longitude;
  //   "datetime": "2018-05-23"
  String observationTimeText;
  //   "datetimeEpoch":1527066000,
  uint32_t observationTime;
  //   "temp":17.35,
  float temp;
  //   "tempmin":16.89, (forecast only)
  float tempMin;
  //   "tempmax":17.35, (forecast only)
  float tempMax;
  //   "humidity":97,
  float humidity;
  // "windspeed":1.77,
  float windSpeed;
  //  "winddir":207.501
  float windDeg;
  //   "pressure":970.8,
  float pressure;
  //   "cloudcover": 44.4,
  float clouds;
  // "sunriseEpoch": 1526960448,
  uint32_t sunrise;
  // "sunsetEpoch": 1527015901,
  uint32_t sunset;
  //   "conditions":"Partially cloudy",
  String conditions;
  //   "description":"Partly cloudy throughout the day.", (forecast only)
  String description;
  //   "icon":"partly-cloudy-day",
  String icon;
  String iconMeteoCon;
} VisualCrossingData;

class VisualCrossing: public JsonListener {
  private:
    const String host = "weather.visualcrossing.com";
    const uint16_t port = 443;
    String currentKey;
    String currentParent = "";
    VisualCrossingData *forecastData;
    VisualCrossingData *currentData;
    uint8_t maxForecasts;
    uint8_t currentForecast;
    boolean metric = true;
    String language = "en";
    boolean isCurrentForecastAllowed = true;

    uint8_t doUpdate(VisualCrossingData *forecastData, VisualCrossingData &currentData, String path);
    String buildPath(String key, String locationParameter);

  public:
    VisualCrossing();
    uint8_t updateForecasts(VisualCrossingData *forecastData, VisualCrossingData &currentData, String key, String location, uint8_t maxForecasts);

    void setMetric(boolean metric) { this->metric = metric; }
    boolean isMetric() { return this->metric; }
    void setLanguage(String language) { this->language = language; }
    String getLanguage() { return this->language; }


    String getMeteoconIcon(String icon, bool night=false);
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
