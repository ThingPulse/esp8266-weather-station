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


#include "WorldClockClient.h"


WorldClockClient::WorldClockClient(String language, String country, String dateFormat, int numberOfTimeZones, String* timeZoneIds) {
  myLanguage = language;
  myCountry = country;
  myDateFormat = dateFormat;
  myNumberOfTimeZoneIds = numberOfTimeZones;
  myTimeZoneIds = timeZoneIds;
  timeZoneOffsetToUtcMillis = (long*) malloc(numberOfTimeZones * sizeof(long));
}

void WorldClockClient::updateTime() {
  JsonStreamingParser parser;
  parser.setListener(this);
  WiFiClient client;

  // http://api.thingspeak.com/channels/CHANNEL_ID/feeds.json?results=2&api_key=API_KEY
  const char host[] = "oleddisplay.squix.ch";
  String url = "/rest/time";

  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }


  Serial.print("Requesting URL: ");
  Serial.println(url);

  // {"language":"de","country":"CH","timeZoneIds":["Europe/Zurich", "Europe/London"],"dateFormat":"dd.MM.YYYY"}
  String timeZoneIdJson = "\"timeZoneIds\":[";
  for (int i = 0; i < myNumberOfTimeZoneIds; i++) {
    if (i > 0) {
      timeZoneIdJson +=",";
    }
    timeZoneIdJson += "\"" + myTimeZoneIds[i] + "\"";
  }
  timeZoneIdJson += "]";
  String request = "{\"language\":\""
      + myLanguage + "\",\"country\":\""
      + myCountry + "\","
      + timeZoneIdJson +",\"dateFormat\":\""
      + myDateFormat +"\"}\r\n\r\n";
  Serial.println("Request: " + request);
  // This will send the request to the server
  client.print("POST " + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Length: " + String(request.length()) + "\r\n" +
               "Connection: close\r\n\r\n");

  client.println(request);

  int retryCounter = 0;
  while(!client.available()) {
    Serial.println(".");
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


String WorldClockClient::getFormattedTime(int timeZoneIndex) {
  return getHours(timeZoneIndex) + ":" + getMinutes(timeZoneIndex) + ":" + getSeconds(timeZoneIndex);
}

String WorldClockClient::getHours(int timeZoneIndex) {
  if (millisOfDayAtUpdate == 0) {
    return "--";
  }
  int hours = ((getSecondsOfDay(timeZoneIndex)  % 86400L) / 3600) % 24;
  if (hours < 10) {
    return "0" + String(hours);
  }
  return String(hours); // print the hour (86400 equals secs per day)
}

String WorldClockClient::getMinutes(int timeZoneIndex) {
  if (millisOfDayAtUpdate == 0) {
    return "--";
  }
  int minutes = ((getSecondsOfDay(timeZoneIndex) % 3600) / 60);
  if (minutes < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    return "0" + String(minutes);
  }
  return String(minutes);
}

String WorldClockClient::getSeconds(int timeZoneIndex) {
  if (millisOfDayAtUpdate == 0) {
    return "--";
  }
  int seconds = getSecondsOfDay(timeZoneIndex) % 60;
  if ( seconds < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    return "0" + String(seconds);
  }
  return String(seconds);

}

long WorldClockClient::getSecondsOfDay(int timeZoneIndex) {
  return (millisOfDayAtUpdate + millis() - localMillisAtUpdate + timeZoneOffsetToUtcMillis[timeZoneIndex]) / 1000;
}

void WorldClockClient::whitespace(char c) {

}

void WorldClockClient::startDocument() {

}

void WorldClockClient::key(String key) {
  currentKey = key;
}

void WorldClockClient::value(String value) {
  Serial.println(currentKey + ": " + value);
  if (currentKey == "millisOfDayUtc") {
    millisOfDayAtUpdate = value.toInt();
    localMillisAtUpdate = millis();
  } else if (currentKey == "index") {
    currentTimeZoneIndex = value.toInt();
    Serial.println("\n-->Current index: " + String(currentTimeZoneIndex));
  } else if (currentKey == "timeZoneOffsetToUtcMillis") {
    Serial.println("\n-->Index: " + String(currentTimeZoneIndex));
    Serial.println("\n-->value: " + value);
    timeZoneOffsetToUtcMillis[currentTimeZoneIndex] = value.toInt();
  }
}

void WorldClockClient::endArray() {

}

void WorldClockClient::endObject() {

}

void WorldClockClient::endDocument() {

}

void WorldClockClient::startArray() {

}

void WorldClockClient::startObject() {

}
