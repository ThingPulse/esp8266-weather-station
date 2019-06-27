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

typedef struct OpenWeatherMapForecastDailyData {
  // {"dt":1527066000,
  uint32_t observationTime;
  // "main":{
  //   "day":17.35,
  float tempDay;
  //   "min":16.89,
  float tempMin;
  //   "max":17.35,
  float tempMax;
  //   "night":17.35,
  float tempNight;
  //   "eve":17.35,
  float tempEve;
  //   "morn":17.35,
  float tempMorn;
  //   "pressure":970.8,
  float pressure;
  //   "humidity":97,
  uint8_t humidity;
  // },"weather":[{
  //   "id":802,
  uint16_t weatherId;
  //   "main":"Clouds",
  String main;
  //   "description":"scattered clouds",
  String description;
  //   "icon":"03d"
  String icon;
  String iconMeteoCon;
  // "clouds":44,
  uint8_t clouds;
  //   "speed":1.77,
  float windSpeed;
  //   "deg":207
  uint8_t windDeg;
  // "rain":2.25,
  float rain;


} OpenWeatherMapForecastDailyData;

class OpenWeatherMapForecastDaily: public JsonListener {
  private:
    String currentKey;
    String currentParent;
    OpenWeatherMapForecastDailyData *data;
    uint8_t weatherItemCounter = 0;
    uint8_t maxForecasts;
    int8_t currentForecast = -1; 
    boolean metric = true;
    String language = "en";
    uint8_t *allowedHours;
    uint8_t allowedHoursCount = 0;
    boolean isCurrentForecastAllowed = true;

    uint8_t doUpdate(OpenWeatherMapForecastDailyData *data, String url);
    String buildUrl(String appId, String locationParameter);

  public:
    OpenWeatherMapForecastDaily();
    uint8_t updateForecasts(OpenWeatherMapForecastDailyData *data, String appId, String location, uint8_t maxForecasts);
    uint8_t updateForecastsById(OpenWeatherMapForecastDailyData *data, String appId, String locationId, uint8_t maxForecasts);

    void setMetric(boolean metric) { this->metric = metric; }
    boolean isMetric() { return this->metric; }
    void setLanguage(String language) { this->language = language; }
    String getLanguage() { return this->language; }
    void setAllowedHours(uint8_t *allowedHours, uint8_t allowedHoursCount) {
      this->allowedHours = allowedHours;
      this->allowedHoursCount = allowedHoursCount;
    }


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
