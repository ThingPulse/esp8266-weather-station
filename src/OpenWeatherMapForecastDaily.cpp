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
#include <ESPHTTPClient.h>
#include "OpenWeatherMapForecastDaily.h"

OpenWeatherMapForecastDaily::OpenWeatherMapForecastDaily() {

}

uint8_t OpenWeatherMapForecastDaily::updateForecasts(OpenWeatherMapForecastDailyData *data, String appId, String location, uint8_t maxForecasts) {
  this->maxForecasts = maxForecasts;
  return doUpdate(data, buildUrl(appId, "q=" + location));
}

uint8_t OpenWeatherMapForecastDaily::updateForecastsById(OpenWeatherMapForecastDailyData *data, String appId, String locationId, uint8_t maxForecasts) {
  this->maxForecasts = maxForecasts;
  return doUpdate(data, buildUrl(appId, "id=" + locationId));
}

String OpenWeatherMapForecastDaily::buildUrl(String appId, String locationParameter) {
  String units = metric ? "metric" : "imperial";
  return "http://api.openweathermap.org/data/2.5/forecast/daily?" + locationParameter + "&appid=" + appId + "&units=" + units + "&lang=" + language + "&cnt=" +maxForecasts;
}

uint8_t OpenWeatherMapForecastDaily::doUpdate(OpenWeatherMapForecastDailyData *data, String url) {
  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();
  this->weatherItemCounter = 0;
  this->currentForecast = -1;
  this->data = data;
  JsonStreamingParser parser;
  parser.setListener(this);
  Serial.printf("Getting url: %s\n", url.c_str());
  HTTPClient http;

  http.begin(url);
  bool isBody = false;
  char c;
  int size;
  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  if(httpCode > 0) {

    WiFiClient * client = http.getStreamPtr();

    while(client->connected() || client->available()) {
      while((size = client->available()) > 0) {
		if ((millis() - lost_do) > lostTest) {
			Serial.println ("lost in client with a timeout");
			client->stop();
			ESP.restart();
	    }
        c = client->read();
        if (c == '{' || c == '[') {

          isBody = true;
        }
        if (isBody) {
          parser.parse(c);
        }
        // give WiFi and TCP/IP libraries a chance to handle pending events
        yield();
      }
    }
  }
  this->data = nullptr;
  return currentForecast;
}

void OpenWeatherMapForecastDaily::whitespace(char c) {
  //Serial.println("whitespace");
}


void OpenWeatherMapForecastDaily::key(String key) {
  currentKey = String(key);
  //Serial.println(currentKey);
}

void OpenWeatherMapForecastDaily::value(String value) {

  if (currentForecast > maxForecasts) {
    return;
  }
  // increment currentForecast when receive the date
  if (currentKey == "dt") {
  	currentForecast++;
  	data[currentForecast].observationTime = value.toInt();
  }
  
  if (currentForecast < 0 ) return;  

  if  ( currentKey == "day" ) {
  	data[currentForecast].tempDay = value.toFloat();
  } else if  ( currentKey == "min" ) {
  	data[currentForecast].tempMin = value.toFloat();
  } else if  ( currentKey == "max" ) {
  	data[currentForecast].tempMax = value.toFloat();
  } else if  ( currentKey == "night" ) {
  	data[currentForecast].tempNight = value.toFloat();
  } else if  ( currentKey == "eve" ) {
  	data[currentForecast].tempEve = value.toFloat();
  } else if  ( currentKey == "morn" ) {
  	data[currentForecast].tempMorn = value.toFloat();
  } else if (currentKey == "pressure") {
    data[currentForecast].pressure = value.toFloat();
  } else if (currentKey == "humidity") {
    data[currentForecast].humidity = value.toFloat();
  } else  if (currentKey == "id") {
      data[currentForecast].weatherId = value.toInt();
  } else if (currentKey == "main") {
      data[currentForecast].main = value;
  } else if (currentKey == "description") {
      data[currentForecast].description = value;
  } else if (currentKey == "icon") {
      data[currentForecast].icon = value;
      data[currentForecast].iconMeteoCon = getMeteoconIcon(value);
  } else if (currentKey == "clouds") {
      data[currentForecast].clouds = value.toInt();
  } else if (currentKey == "speed") {
      data[currentForecast].windSpeed = value.toFloat();
  } else if (currentKey == "deg") {
      data[currentForecast].windDeg = value.toInt();
  } else if (currentKey == "rain") {
      data[currentForecast].rain = value.toFloat();
  } 
  	/*
  // {"dt":1527066000,  uint32_t observationTime;
  if (currentKey == "dt") {
    data[currentForecast].observationTime = value.toInt();

    if (allowedHoursCount > 0) {
      time_t time = data[currentForecast].observationTime;
      struct tm* timeInfo;
      timeInfo = gmtime(&time);
      uint8_t currentHour = timeInfo->tm_hour;
      for (uint8_t i = 0; i < allowedHoursCount; i++) {
        if (currentHour == allowedHours[i]) {
          isCurrentForecastAllowed = true;
          return;
        }
      }
      isCurrentForecastAllowed = false;
      return;
    }
  }
  if (!isCurrentForecastAllowed) {
    return;
  }
  // "main":{
  //   "temp":17.35, float temp;
  if (currentKey == "temp") {
    data[currentForecast].temp = value.toFloat();
    // initialize potentially empty values:
    data[currentForecast].rain = 0;;
  }
  //   "temp_min":16.89, float tempMin;
  if (currentKey == "temp_min") {
    data[currentForecast].tempMin = value.toFloat();
  }
  //   "temp_max":17.35,float tempMax;
  if (currentKey == "temp_max") {
    data[currentForecast].tempMax = value.toFloat();
  }
  //   "pressure":970.8,float pressure;
  if (currentKey == "pressure") {
    data[currentForecast].pressure = value.toFloat();
  }
  //   "sea_level":1030.62,float pressureSeaLevel;
  if (currentKey == "sea_level") {
    data[currentForecast].pressureSeaLevel = value.toFloat();
  }
  //   "grnd_level":970.8,float pressureGroundLevel;
  if (currentKey == "grnd_level") {
    data[currentForecast].pressureGroundLevel = value.toFloat();
  }
  //   "":97,uint8_t humidity;
  if (currentKey == "humidity") {
    data[currentForecast].humidity = value.toInt();
  }
  //   "temp_kf":0.46
  // },"weather":[{

  if (currentParent == "weather") {
    //   "id":802,uint16_t weatherId;
    if (currentKey == "id") {
      data[currentForecast].weatherId = value.toInt();
    }

    //   "main":"Clouds",String main;
    if (currentKey == "main") {
      data[currentForecast].main = value;
    }
    //   "description":"scattered clouds",String description;
    if (currentKey == "description") {
      data[currentForecast].description = value;
    }
    //   "icon":"03d" String icon; String iconMeteoCon;
    if (currentKey == "icon") {
      data[currentForecast].icon = value;
      data[currentForecast].iconMeteoCon = getMeteoconIcon(value);
    }
  }
  // }],"clouds":{"all":44},uint8_t clouds;
  if (currentKey == "all") {
    data[currentForecast].clouds = value.toInt();
  }
  // "wind":{
  //   "speed":1.77, float windSpeed;
  if (currentKey == "speed") {
    data[currentForecast].windSpeed = value.toFloat();
  }
  //   "deg":207.501 float windDeg;
  if (currentKey == "deg") {
    data[currentForecast].windDeg = value.toFloat();
  }
  // rain: {3h: 0.055}, float rain;
  if (currentKey == "3h") {
    data[currentForecast].rain = value.toFloat();
  }
  // },"sys":{"pod":"d"}
  // dt_txt: "2018-05-23 09:00:00"   String observationTimeText;
  if (currentKey == "dt_txt") {
    data[currentForecast].observationTimeText = value;
    // this is not super save, if there is no dt_txt item we'll never get all forecasts;
    currentForecast++;
  }

  */
}

void OpenWeatherMapForecastDaily::startDocument() {
  //Serial.println(">>>");
}


void OpenWeatherMapForecastDaily::endArray() {
	//Serial.println("]");
}


void OpenWeatherMapForecastDaily::startObject() {
  currentParent = currentKey;
  //Serial.println("{");
}

void OpenWeatherMapForecastDaily::endObject() {
  if (currentParent == "weather") {
    weatherItemCounter++;
  }
  currentParent = "";
 // Serial.println("}");
}

void OpenWeatherMapForecastDaily::endDocument() {
	//Serial.println("<<<");
}

void OpenWeatherMapForecastDaily::startArray() {
  //Serial.println("[");
}


String OpenWeatherMapForecastDaily::getMeteoconIcon(String icon) {
 	// clear sky
  // 01d
  if (icon == "01d") 	{
    return "B";
  }
  // 01n
  if (icon == "01n") 	{
    return "C";
  }
  // few clouds
  // 02d
  if (icon == "02d") 	{
    return "H";
  }
  // 02n
  if (icon == "02n") 	{
    return "4";
  }
  // scattered clouds
  // 03d
  if (icon == "03d") 	{
    return "N";
  }
  // 03n
  if (icon == "03n") 	{
    return "5";
  }
  // broken clouds
  // 04d
  if (icon == "04d") 	{
    return "Y";
  }
  // 04n
  if (icon == "04n") 	{
    return "%";
  }
  // shower rain
  // 09d
  if (icon == "09d") 	{
    return "R";
  }
  // 09n
  if (icon == "09n") 	{
    return "8";
  }
  // rain
  // 10d
  if (icon == "10d") 	{
    return "Q";
  }
  // 10n
  if (icon == "10n") 	{
    return "7";
  }
  // thunderstorm
  // 11d
  if (icon == "11d") 	{
    return "P";
  }
  // 11n
  if (icon == "11n") 	{
    return "6";
  }
  // snow
  // 13d
  if (icon == "13d") 	{
    return "W";
  }
  // 13n
  if (icon == "13n") 	{
    return "#";
  }
  // mist
  // 50d
  if (icon == "50d") 	{
    return "M";
  }
  // 50n
  if (icon == "50n") 	{
    return "M";
  }
  // Nothing matched: N/A
  return ")";

}
