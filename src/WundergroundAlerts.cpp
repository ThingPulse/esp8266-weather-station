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

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "WundergroundAlerts.h"

WundergroundAlerts::WundergroundAlerts() {

}
void WundergroundAlerts::updateAlerts(String apiKey, String language, String country, String city) {
  currentAlert = 0;
  activeAlertsCnt = 0;
  isAlerts = true;
  if (country == "US") {
	isAlertUS = true;
	isAlertEU = false;
  } else {
	isAlertUS = false;
	isAlertEU = true;
  }
  doUpdate("/api/" + apiKey + "/alerts/lang:" + language + "/q/" + country + "/" + city + ".json");
}
// end fowlerk add

void WundergroundAlerts::updateAlertsPWS(String apiKey, String language, String country, String pws) {
  currentAlert = 0;
  activeAlertsCnt = 0;
  isAlerts = true;
  if (country == "US") {
    isAlertUS = true;
    isAlertEU = false;
  } else {
    isAlertUS = false;
    isAlertEU = true;
  }
  doUpdate("/api/" + apiKey + "/alerts/lang:" + language + "/q/pws:" + pws + ".json");
}

void WundergroundAlerts::doUpdate(String url) {
  JsonStreamingParser parser;
  parser.setListener(this);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("api.wunderground.com", httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.wunderground.com\r\n" +
               "Connection: close\r\n\r\n");
  int retryCounter = 0;
  while(!client.available()) {
    delay(1000);
    retryCounter++;
    if (retryCounter > 10) {
      return;
    }
  }

  int pos = 0;
  boolean isBody = false;
  char c;

  int size = 0;
  client.setNoDelay(false);
  while(client.connected()) {
    while((size = client.available()) > 0) {
      c = client.read();
      if (c == '{' || c == '[') {
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
      }
    }
  }
}

void WundergroundAlerts::whitespace(char c) {
  Serial.println("whitespace");
}

void WundergroundAlerts::startDocument() {
  Serial.println("start document");
}

void WundergroundAlerts::key(String key) {
  currentKey = String(key);
}

void WundergroundAlerts::value(String value) {
  if (currentKey == "type" && isAlerts) {
  	activeAlertsCnt++;
  	currentAlert++;
  	activeAlerts[currentAlert-1] = value;
  	Serial.print("Alert type processed, value:  "); Serial.println(activeAlerts[currentAlert-1]);
  }
  if (currentKey == "description" && isAlerts && isAlertUS) {
    activeAlertsText[currentAlert-1] = value;
  	Serial.print("Alert description processed, value:  "); Serial.println(activeAlertsText[currentAlert-1]);
  }
  if (currentKey == "wtype_meteoalarm_name" && isAlerts && isAlertEU) {
    activeAlertsText[currentAlert-1] = value;
    Serial.print("Alert description processed, value:  "); Serial.println(activeAlertsText[currentAlert-1]);
  }
  if (currentKey == "message" && isAlerts) {
    activeAlertsMessage[currentAlert-1] = value;
    Serial.print("Alert msg length:  "); Serial.println(activeAlertsMessage[currentAlert-1].length());
    if(activeAlertsMessage[currentAlert-1].length() >= 511) {
      activeAlertsMessageTrunc[currentAlert-1] = true;
  	} else {
  		activeAlertsMessageTrunc[currentAlert-1] = false;
  	}
	   Serial.print("Alert message processed, value:  "); Serial.println(activeAlertsMessage[currentAlert-1]);
  }
  if (currentKey == "date" && isAlerts) {
      activeAlertsStart[currentAlert-1] = value;
	    // Check last char for a "/"; the returned value sometimes includes this; if so, strip it (47 is a "/" char)
      if (activeAlertsStart[currentAlert-1].charAt(activeAlertsStart[currentAlert-1].length()-1) == 47) {
        Serial.println("...last char is a slash...");
        activeAlertsStart[currentAlert-1] = activeAlertsStart[currentAlert-1].substring(0,(activeAlertsStart[currentAlert-1].length()-1));
      }
    	// For meteoalarms, the start field is returned with the UTC=0 by default (not used?)
    	if (isAlertEU && activeAlertsStart[currentAlert-1] == "1970-01-01 00:00:00 GMT") {
    		activeAlertsStart[currentAlert-1] = "<Not specified>";
    	}
      Serial.print("Alert start processed, value:  "); Serial.println(activeAlertsStart[currentAlert-1]);
  }
  if (currentKey == "expires" && isAlerts) {
    activeAlertsEnd[currentAlert-1] = value;
    Serial.print("Alert expiration processed, value:  "); Serial.println(activeAlertsEnd[currentAlert-1]);
  }
  if (currentKey == "phenomena" && isAlerts) {
    activeAlertsPhenomena[currentAlert-1] = value;
    Serial.print("Alert phenomena processed, value:  "); Serial.println(activeAlertsPhenomena[currentAlert-1]);
  }
  if (currentKey == "significance" && isAlerts && isAlertUS) {
    activeAlertsSignificance[currentAlert-1] = value;
    Serial.print("Alert significance processed, value:  "); Serial.println(activeAlertsSignificance[currentAlert-1]);
  }
  // Map meteoalarm level to the field for significance for consistency (used for European alerts)
  if (currentKey == "level_meteoalarm" && isAlerts && isAlertEU) {
    activeAlertsSignificance[currentAlert-1] = value;
    Serial.print("Meteo alert significance processed, value:  "); Serial.println(activeAlertsSignificance[currentAlert-1]);
  }
  // For meteoalarms only (European alerts); attribution must be displayed according to the T&C's of use
  if (currentKey == "attribution" && isAlerts) {
  	activeAlertsAttribution[currentAlert-1] = value;
  	// Remove some of the markup in the attribution
  	activeAlertsAttribution[currentAlert-1].replace(" <a href='"," ");
  	activeAlertsAttribution[currentAlert-1].replace("</a>","");
  	activeAlertsAttribution[currentAlert-1].replace("/'>"," ");
  }

}

void WundergroundAlerts::endArray() {

}


void WundergroundAlerts::startObject() {
  currentParent = currentKey;
}

void WundergroundAlerts::endObject() {
  currentParent = "";
}

void WundergroundAlerts::endDocument() {

}

void WundergroundAlerts::startArray() {

}

String WundergroundAlerts::getActiveAlerts(int alertIndex) {
  return activeAlerts[alertIndex];
}

String WundergroundAlerts::getActiveAlertsText(int alertIndex) {
  return activeAlertsText[alertIndex];
}

String WundergroundAlerts::getActiveAlertsMessage(int alertIndex) {
  return activeAlertsMessage[alertIndex];
}

bool WundergroundAlerts::getActiveAlertsMessageTrunc(int alertIndex) {
  return activeAlertsMessageTrunc[alertIndex];
}

String WundergroundAlerts::getActiveAlertsStart(int alertIndex) {
  return activeAlertsStart[alertIndex];
}

String WundergroundAlerts::getActiveAlertsEnd(int alertIndex) {
  return activeAlertsEnd[alertIndex];
}

String WundergroundAlerts::getActiveAlertsPhenomena(int alertIndex) {
  return activeAlertsPhenomena[alertIndex];
}

String WundergroundAlerts::getActiveAlertsSignificance(int alertIndex) {
  return activeAlertsSignificance[alertIndex];
}

String WundergroundAlerts::getActiveAlertsAttribution(int alertIndex) {
  return activeAlertsAttribution[alertIndex];
}

int WundergroundAlerts::getActiveAlertsCnt() {
  return activeAlertsCnt;
}
