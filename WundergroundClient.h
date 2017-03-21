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

class WundergroundClient: public JsonListener {
  private:
    String currentKey;
    String currentParent = "";
    long localEpoc = 0;
    int gmtOffset = 1;
    long localMillisAtUpdate;
    String date = "-";
    String observationDate = "-";
    boolean isMetric = true;
    String currentTemp;
    // JJG added ... ////////////////////////////////// define returns /////////////////////////////////
    String moonPctIlum;  // not used
    String moonAge;      // make this a long?
    String moonPhase;
    String sunriseTime;
    String sunsetTime;
    String moonriseTime;
    String moonsetTime;
    String windSpeed;
    String windDir;
    // end JJG add ////////////////////////////////////////////////////////////////////////////////////
    String weatherIcon;
    String weatherText;
    String humidity;
    String pressure;
    String dewPoint;
    String precipitationToday;
	// fowlerk added...
	String feelslike;
	String UV;
	String observationTime;					// fowlerk add, 04-Dec-2016
	// end fowlerk add

    void doUpdate(String url);

    // forecast
    boolean isForecast = false;
    boolean isSimpleForecast = false;		// true;  fowlerk
	boolean isCurrentObservation = false;	// Added by fowlerk
	boolean isAlerts = false;				// Added by fowlerk
	boolean isAlertUS = false;				// Added by fowlerk
	boolean isAlertEU = false;				// Added by fowlerk
    int currentForecastPeriod;
    String forecastIcon [MAX_FORECAST_PERIODS];
    String forecastTitle [MAX_FORECAST_PERIODS];
    String forecastLowTemp [MAX_FORECAST_PERIODS];
    String forecastHighTemp [MAX_FORECAST_PERIODS];
	// fowlerk added...
	String forecastDay [MAX_FORECAST_PERIODS/2];
	String forecastMonth [MAX_FORECAST_PERIODS/2];
	String forecastText [MAX_FORECAST_PERIODS];
	String PoP [MAX_FORECAST_PERIODS];
	// Active alerts...added 18-Dec-2016
	String activeAlerts [MAX_WEATHER_ALERTS];			   // For a max of 6 currently-active alerts
	String activeAlertsMessage [MAX_WEATHER_ALERTS];	   // Alert full-text message
	bool   activeAlertsMessageTrunc [MAX_WEATHER_ALERTS];  // Alert full-text message truncation flag
	String activeAlertsText [MAX_WEATHER_ALERTS];		   // Alerts description text
	String activeAlertsStart [MAX_WEATHER_ALERTS];		   // Start of alert date/time
	String activeAlertsEnd [MAX_WEATHER_ALERTS];		   // Expiration of alert date/time
	String activeAlertsPhenomena [MAX_WEATHER_ALERTS];	   // Alert phenomena code
	String activeAlertsSignificance [MAX_WEATHER_ALERTS];  // Alert significance code
	String activeAlertsAttribution [MAX_WEATHER_ALERTS];   // Alert significance code
	int activeAlertsCnt;				   				   // Number of active alerts
	int currentAlert;					   				   // For indexing the current active alert
	// end fowlerk add

  public:
    WundergroundClient(boolean isMetric);
    void updateConditions(String apiKey, String language, String country, String city);
    void updateConditions(String apiKey, String language, String zmwCode);
    void updateConditionsPWS(String apiKey, String language, String pws);
    void updateForecast(String apiKey, String language, String country, String city);
    void updateForecastPWS(String apiKey, String language, String pws);
    void updateForecastZMW(String apiKey, String language, String zmwCode);
    void updateAstronomy(String apiKey, String language, String country, String city);
    void updateAstronomyPWS(String apiKey, String language, String pws);
	  void updateAlerts(String apiKey, String language, String country, String city);		// Added by fowlerk, 18-Dec-2016
	  void updateAlertsPWS(String apiKey, String language, String country, String pws);
    void initMetric(boolean isMetric);			// Added by fowlerk, 12/22/16, as an option to change metric setting other than at instantiation
    // JJG added
    String getHours();
    String getMinutes();
    String getSeconds();
    String getDate();
    String getObservationDate();
    // JJG added ... ///////////////////function name to string ////////////////////////////
    String getMoonPctIlum();
    String getMoonAge();
    String getMoonPhase();
    String getSunriseTime();
    String getSunsetTime();
    String getMoonriseTime();
    String getMoonsetTime();
    String getWindSpeed();
    String getWindDir();
    // end JJG add ///////////////////////////////////////////////////////////////////////
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
	  // fowlerk added...
	  String getFeelsLike();

	  String getUV();

	  String getObservationTime();			// fowlerk add, 04-Dec-2016
	  // end fowlerk add

    String getForecastIcon(int period);

    String getForecastTitle(int period);

    String getForecastLowTemp(int period);

    String getForecastHighTemp(int period);
	  // fowlerk added...
	  String getForecastDay(int period);

	  String getForecastMonth(int period);

	  String getForecastText(int period);

	  String getPoP(int period);

	  int getActiveAlertsCnt();

	  String getActiveAlerts(int alertIndex);

	  String getActiveAlertsText(int alertIndex);

	  String getActiveAlertsMessage(int alertIndex);

	  bool getActiveAlertsMessageTrunc(int alertIndex);

	  String getActiveAlertsStart(int alertIndex);

	  String getActiveAlertsEnd(int alertIndex);

	  String getActiveAlertsPhenomena(int alertIndex);

	  String getActiveAlertsSignificance(int alertIndex);

	  String getActiveAlertsAttribution(int alertIndex);

	  // end fowlerk add

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
