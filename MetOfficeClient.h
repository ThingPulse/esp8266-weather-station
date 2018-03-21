/* The MIT License (MIT)
*
*   Copyright (c) 2017 by Will Jenkins
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
*   Based on WundergroundClient created by Daniel Eichhorn:
*   https://github.com/squix78/esp8266-weather-station
*/

#pragma once

#include <JsonListener.h>
#include <JsonStreamingParser.h>

class MetOfficeClient: public JsonListener {
  private:
    String currentKey;
    String currentParent = "";
    String currentDate = "";
    String currentForecast = "";
    String secondForecast = "";
    String thirdForecast = "";
    String fourthForecast = "";
    String locationName = "";
    String temperature = "";
    String feelsLikeTemp = "";
    String windDirection = "";
    String windSpeed = "";
    String windGust = "";
    String maxUvIndex = "";
    String weatherType = "";
    String precipProb = "";
    String forecastArray[36] = { }; // String array to hold the segmented forecast data for the required 4 periods
    int count = 0; // Count used to populate the above array with required forecast values on parsing

    void doUpdate(String url);

  public:
    MetOfficeClient();
    virtual void whitespace(char c);
    virtual void startDocument();
    virtual void key(String key);
    virtual void value(String value);
    virtual void endArray();
    virtual void endObject();
    virtual void endDocument();
    virtual void startArray();
    virtual void startObject();

    void updateConditions(String ThirdForecastPeriod, int location, String apiKey);

    String getLocationName();
    String getCurrentDate();
    String getCurrentForecastTime();
    String getCurrentWindDirection();
    String getCurrentFeelsLikeTemp();
    String getCurrentWindGust();
    String getCurrentPrecipProb();
    String getCurrentWindSpeed();
    String getCurrentTemperature();
    String getCurrentWeatherType();
    String getCurrentMaxUvIndex();
    String getSecondForecastTime();
    String getSecondWindDirection();
    String getSecondFeelsLikeTemp();
    String getSecondWindGust();
    String getSecondPrecipProb();
    String getSecondWindSpeed();
    String getSecondTemperature();
    String getSecondWeatherType();
    String getSecondMaxUvIndex();
    String getThirdForecastTime();
    String getThirdWindDirection();
    String getThirdFeelsLikeTemp();
    String getThirdWindGust();
    String getThirdPrecipProb();
    String getThirdWindSpeed();
    String getThirdTemperature();
    String getThirdWeatherType();
    String getThirdMaxUvIndex();
    String getFourthForecastTime();
    String getFourthWindDirection();
    String getFourthFeelsLikeTemp();
    String getFourthWindGust();
    String getFourthPrecipProb();
    String getFourthWindSpeed();
    String getFourthTemperature();
    String getFourthWeatherType();
    String getFourthMaxUvIndex();
    String getWeatherIconName(String weatherType);

};
