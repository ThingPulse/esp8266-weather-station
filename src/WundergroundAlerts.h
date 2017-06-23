/**The MIT License (MIT)

Copyright (c) 2017 by Daniel Eichhorn

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

#define MAX_WEATHER_ALERTS 6  	 // The maximum number of concurrent weather alerts supported by the library

class WundergroundAlerts: public JsonListener {
  private:
    String currentKey;
    String currentParent = "";


    void doUpdate(String url);

	boolean isAlerts = false;				// Added by fowlerk
	boolean isAlertUS = false;				// Added by fowlerk
	boolean isAlertEU = false;				// Added by fowlerk
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
    WundergroundAlerts();
	  void updateAlerts(String apiKey, String language, String country, String city);		// Added by fowlerk, 18-Dec-2016
	  void updateAlertsPWS(String apiKey, String language, String country, String pws);
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
