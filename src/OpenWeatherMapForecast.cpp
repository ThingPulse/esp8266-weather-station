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
#include "OpenWeatherMapForecast.h"

OpenWeatherMapForecast::OpenWeatherMapForecast() {

}

uint8_t OpenWeatherMapForecast::updateForecasts(OpenWeatherMapForecastData *data, String appId, String location, uint8_t maxForecasts) {
  this->maxForecasts = maxForecasts;
  return doUpdate(data, buildPath(appId, "q=" + location));
}

uint8_t OpenWeatherMapForecast::updateForecastsById(OpenWeatherMapForecastData *data, String appId, String locationId, uint8_t maxForecasts) {
  this->maxForecasts = maxForecasts;
  return doUpdate(data, buildPath(appId, "id=" + locationId));
}

String OpenWeatherMapForecast::buildPath(String appId, String locationParameter) {
  String units = metric ? "metric" : "imperial";
  return "/data/2.5/forecast?" + locationParameter + "&appid=" + appId + "&units=" + units + "&lang=" + language;
}

uint8_t OpenWeatherMapForecast::doUpdate(OpenWeatherMapForecastData *data, String path) {
  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();
  this->weatherItemCounter = 0;
  this->currentForecast = 0;
  this->data = data;
  JsonStreamingParser parser;
  parser.setListener(this);
  Serial.printf("[HTTP] Requesting resource at http://%s:%u%s\n", host.c_str(), port, path.c_str());

  WiFiClient client;
  if(client.connect(host, port)) {
    bool isBody = false;
    char c;
    Serial.println("[HTTP] connected, now GETting data");
    client.print("GET " + path + " HTTP/1.1\r\n"
                 "Host: " + host + "\r\n"
                 "Connection: close\r\n\r\n");

    while (client.connected() || client.available()) {
      if (client.available()) {
        if ((millis() - lost_do) > lostTest) {
          Serial.println("[HTTP] lost in client with a timeout");
          client.stop();
          ESP.restart();
        }
        c = client.read();
        if (c == '{' || c == '[') {
          isBody = true;
        }
        if (isBody) {
          parser.parse(c);
        }
      }
      // give WiFi and TCP/IP libraries a chance to handle pending events
      yield();
    }
    client.stop();
  } else {
    Serial.println("[HTTP] failed to connect to host");
  }
  this->data = nullptr;
  return currentForecast;
}

void OpenWeatherMapForecast::whitespace(char c) {
  Serial.println("whitespace");
}

void OpenWeatherMapForecast::startDocument() {
  Serial.println("start document");
}

void OpenWeatherMapForecast::key(String key) {
  currentKey = String(key);
}

void OpenWeatherMapForecast::value(String value) {
  if (currentForecast >= maxForecasts) {
    return;
  }
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
  //   "feels_like": 16.99, float feelsLike;
  if (currentKey == "feels_like") {
    data[currentForecast].feelsLike = value.toFloat();
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
}

void OpenWeatherMapForecast::endArray() {

}


void OpenWeatherMapForecast::startObject() {
  currentParent = currentKey;
}

void OpenWeatherMapForecast::endObject() {
  if (currentParent == "weather") {
    weatherItemCounter++;
  }
  currentParent = "";
}

void OpenWeatherMapForecast::endDocument() {

}

void OpenWeatherMapForecast::startArray() {

}


String OpenWeatherMapForecast::getMeteoconIcon(String icon) {
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
