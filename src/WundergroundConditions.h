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

#pragma once

#include <JsonListener.h>
#include <JsonStreamingParser.h>

#define MAX_FORECAST_PERIODS 20  // Changed from 7 to 12 to support 6 day / 2 screen forecast (Neptune)
								 // Changed to 20 to support max 10-day forecast returned from 'forecast10day' API (fowlerk)

#define MAX_WEATHER_ALERTS 6  	 // The maximum number of concurrent weather alerts supported by the library

class WundergroundConditions: public JsonListener {
  private:
    String currentKey;
    String currentParent = "";

    String date = "-";
    String observationDate = "-";
    boolean isMetric = true;
    String currentTemp;

    String windSpeed;
    String windDir;
    String weatherIcon;
    String weatherText;
    String humidity;
    String pressure;
    String dewPoint;
    String precipitationToday;
    String feelslike;
    String UV;
    String observationTime;
    boolean usePM;
    boolean isPM;

  void doUpdate(String url);

  public:
    WundergroundConditions(boolean isMetric, boolean usePM);
    void updateConditions(String apiKey, String language, String country, String city);
    void updateConditions(String apiKey, String language, String zmwCode);
    void updateConditionsPWS(String apiKey, String language, String pws);
    String getDate();
    String getObservationDate();
    String getWindSpeed();
    String getWindDir();
    long getCurrentEpoch();
    String getCurrentTemp();
    String getTodayIcon();
    String getTodayIconText();
    String getMeteoconIcon(String iconText);
    String getWeatherText();
    String getHumidity();
    String getPressure();
    String getDewPoint();
    String getPrecipitationToday();
	  String getFeelsLike();
	  String getUV();
	  String getObservationTime();

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
