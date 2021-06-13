/**The MIT License (MIT)
 
 Copyright (c) 2020 by Chris Klinger
 
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
#include "OpenWeatherMapOneCall.h"

String PATH_SEPERATOR = "/";

OpenWeatherMapOneCall::OpenWeatherMapOneCall()
{
}

void OpenWeatherMapOneCall::update(OpenWeatherMapOneCallData *data, String appId, float lat, float lon)
{
  doUpdate(data, buildPath(appId, lat, lon));
}

String OpenWeatherMapOneCall::buildPath(String appId, float lat, float lon)
{
  String units = metric ? "metric" : "imperial";
  return "/data/2.5/onecall?appid=" + appId + "&lat=" + lat + "&lon=" + lon + "&units=" + units + "&lang=" + language;
}

void OpenWeatherMapOneCall::doUpdate(OpenWeatherMapOneCallData *data, String path)
{
  unsigned long lostTest = 10000UL;
  unsigned long lost_do = millis();
  this->weatherItemCounter = 0;
  this->hourlyItemCounter = 0;
  this->dailyItemCounter = 0;
  this->data = data;
  JsonStreamingParser parser;
  parser.setListener(this);
  Serial.printf("[HTTP] Requesting resource at http://%s:%u%s\n", host.c_str(), port, path.c_str());

  WiFiClient client;
  #if defined(ESP8266)
  if (client.connect(host, port)) {
  #else
  if (client.connect(host.c_str(), port)) {
  #endif
    bool isBody = false;
    char c;
    Serial.println("[HTTP] connected, now GETting data");
    client.print("GET " + path + " HTTP/1.1\r\n"
                                 "Host: " +
                 host + "\r\n"
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
}

void OpenWeatherMapOneCall::whitespace(char c)
{
  Serial.println("whitespace");
}

void OpenWeatherMapOneCall::startDocument()
{
  Serial.println("start document");
}

void OpenWeatherMapOneCall::key(String key)
{
  currentKey = key;
}

void OpenWeatherMapOneCall::value(String value)
{
  // "lon": 8.54, float lon;
  if (currentKey == "lon") {
    this->data->lon = value.toFloat();
  }
  // "lat": 47.37 float lat;
  if (currentKey == "lat") {
    this->data->lat = value.toFloat();
  }
  // "timezone": "Europe/Berlin"
  if (currentKey == "timezone") {
    this->data->timezone = value;
  }

  // "current": {..}
  if (currentParent.startsWith("/ROOT/current")) {
    // "current": {.. "dt": 1587216739, .. }
    if (currentKey == "dt") {
      this->data->current.dt = value.toInt();
    }
    if (currentKey == "sunrise") {
      this->data->current.sunrise = value.toInt();
    }
    if (currentKey == "sunset") {
      this->data->current.sunset = value.toInt();
    }
    if (currentKey == "temp") {
      this->data->current.temp = value.toFloat();
    }
    if (currentKey == "feels_like") {
      this->data->current.feels_like = value.toFloat();
    }
    if (currentKey == "pressure") {
      this->data->current.pressure = value.toInt();
    }
    if (currentKey == "humidity") {
      this->data->current.humidity = value.toInt();
    }
    if (currentKey == "dew_point") {
      this->data->current.dew_point = value.toFloat();
    }
    if (currentKey == "uvi") {
      this->data->current.uvi = value.toFloat();
    }
    if (currentKey == "clouds") {
      this->data->current.clouds = value.toInt();
    }
    if (currentKey == "visibility") {
      this->data->current.visibility = value.toInt();
    }
    if (currentKey == "wind_speed") {
      this->data->current.windSpeed = value.toFloat();
    }
    if (currentKey == "wind_deg") {
      this->data->current.windDeg = value.toFloat();
    }
    // weatherItemCounter: only get the first item if more than one is available
    if (currentParent.startsWith("/ROOT/current/weather[]") && weatherItemCounter == 0) {
      if (currentKey == "id") {
        this->data->current.weatherId = value.toInt();
      }
      if (currentKey == "main") {
        this->data->current.weatherMain = value;
      }
      if (currentKey == "description") {
        this->data->current.weatherDescription = value;
      }
      if (currentKey == "icon") {
        this->data->current.weatherIcon = value;
        this->data->current.weatherIconMeteoCon = getMeteoconIcon(value);
      }
    }
  }

  // "hourly": [..]
  if(currentParent.startsWith("/ROOT/hourly[]")) {
    if (currentKey == "dt") {
      this->data->hourly[hourlyItemCounter].dt = value.toInt();
    }
    if (currentKey == "temp") {
      this->data->hourly[hourlyItemCounter].temp = value.toFloat();
    }
    if (currentKey == "feels_like") {
      this->data->hourly[hourlyItemCounter].feels_like = value.toFloat();
    }
    if (currentKey == "pressure") {
      this->data->hourly[hourlyItemCounter].pressure = value.toInt();
    }
    if (currentKey == "humidity") {
      this->data->hourly[hourlyItemCounter].humidity = value.toInt();
    }
    if (currentKey == "dew_point") {
      this->data->hourly[hourlyItemCounter].dew_point = value.toFloat();
    }
    if (currentKey == "clouds") {
      this->data->hourly[hourlyItemCounter].clouds = value.toInt();
    }
    if (currentKey == "wind_speed") {
      this->data->hourly[hourlyItemCounter].windSpeed = value.toFloat();
    }
    if (currentKey == "wind_deg") {
      this->data->hourly[hourlyItemCounter].windDeg = value.toFloat();
    }

    // weatherItemCounter: only get the first item if more than one is available
    if (currentParent.startsWith("/ROOT/hourly[]/_obj/weather[]") && weatherItemCounter == 0) {
      if (currentKey == "id") {
        this->data->hourly[hourlyItemCounter].weatherId = value.toInt();
      }
      if (currentKey == "main") {
        this->data->hourly[hourlyItemCounter].weatherMain = value;
      }
      if (currentKey == "description") {
        this->data->hourly[hourlyItemCounter].weatherDescription = value;
      }
      if (currentKey == "icon") {
        this->data->hourly[hourlyItemCounter].weatherIcon = value;
        this->data->hourly[hourlyItemCounter].weatherIconMeteoCon = getMeteoconIcon(value);
      }
    }
  }

  // "daily": [..]
  if(currentParent.startsWith("/ROOT/daily[]")) {
    if (currentKey == "dt") {
      this->data->daily[dailyItemCounter].dt = value.toInt();
    }
    if (currentKey == "sunrise") {
      this->data->daily[dailyItemCounter].sunrise = value.toInt();
    }
    if (currentKey == "sunset") {
      this->data->daily[dailyItemCounter].sunset = value.toInt();
    }
    if (currentKey == "pressure") {
      this->data->daily[dailyItemCounter].pressure = value.toInt();
    }
    if (currentKey == "humidity") {
      this->data->daily[dailyItemCounter].humidity = value.toInt();
    }
    if (currentKey == "dew_point") {
      this->data->daily[dailyItemCounter].dew_point = value.toFloat();
    }
    if (currentKey == "wind_speed") {
      this->data->daily[dailyItemCounter].windSpeed = value.toFloat();
    }
    if (currentKey == "wind_deg") {
      this->data->daily[dailyItemCounter].windDeg = value.toFloat();
    }
    if (currentKey == "clouds") {
      this->data->daily[dailyItemCounter].clouds = value.toInt();
    }
    if (currentKey == "rain") {
      this->data->daily[dailyItemCounter].rain = value.toFloat();
    }
    if (currentKey == "snow") {
      this->data->daily[dailyItemCounter].snow = value.toFloat();
    }
    if (currentKey == "uvi") {
      this->data->daily[dailyItemCounter].uvi = value.toFloat();
    }

    if (currentParent.startsWith("/ROOT/daily[]/_obj/temp")) {
      if (currentKey == "day") {
        this->data->daily[dailyItemCounter].tempDay = value.toFloat();
      }
      if (currentKey == "min") {
        this->data->daily[dailyItemCounter].tempMin = value.toFloat();
      }
      if (currentKey == "max") {
        this->data->daily[dailyItemCounter].tempMax = value.toFloat();
      }
      if (currentKey == "night") {
        this->data->daily[dailyItemCounter].tempNight = value.toFloat();
      }
      if (currentKey == "eve") {
        this->data->daily[dailyItemCounter].tempEve = value.toFloat();
      }
      if (currentKey == "morn") {
        this->data->daily[dailyItemCounter].tempMorn = value.toFloat();
      }
    }
  
    if (currentParent.startsWith("/ROOT/daily[]/_obj/feels_like")) {
      if (currentKey == "day") {
        this->data->daily[dailyItemCounter].feels_likeDay = value.toFloat();
      }
      if (currentKey == "night") {
        this->data->daily[dailyItemCounter].feels_likeNight = value.toFloat();
      }
      if (currentKey == "eve") {
        this->data->daily[dailyItemCounter].feels_likeEve = value.toFloat();
      }
      if (currentKey == "morn") {
        this->data->daily[dailyItemCounter].feels_likeMorn = value.toFloat();
      }
    }
    // weatherItemCounter: only get the first item if more than one is available
    if (currentParent.startsWith("/ROOT/daily[]/_obj/weather[]") && weatherItemCounter == 0) {
      if (currentKey == "id") {
        this->data->daily[dailyItemCounter].weatherId = value.toInt();
      }
      if (currentKey == "main") {
        this->data->daily[dailyItemCounter].weatherMain = value;
      }
      if (currentKey == "description") {
        this->data->daily[dailyItemCounter].weatherDescription = value;
      }
      if (currentKey == "icon") {
        this->data->daily[dailyItemCounter].weatherIcon = value;
        this->data->daily[dailyItemCounter].weatherIconMeteoCon = getMeteoconIcon(value);
      }
    }
  }
}

void OpenWeatherMapOneCall::endArray()
{
  currentKey = "";
  currentParent= currentParent.substring(0, currentParent.lastIndexOf(PATH_SEPERATOR));
}

void OpenWeatherMapOneCall::startObject()
{
  if(currentKey == "") {
    currentKey = "_obj";
  }
  currentParent += PATH_SEPERATOR + currentKey;
}

void OpenWeatherMapOneCall::endObject()
{
  if (currentParent == "/ROOT/current/weather[]/_obj" || currentParent == "/ROOT/daily[]/_obj/weather[]/_obj" || currentParent == "/ROOT/daily[]/_obj/weather[]/_obj"  ) {
    weatherItemCounter++;
  }
  if (currentParent == "/ROOT/hourly[]/_obj") {
    hourlyItemCounter++;
  }
  if (currentParent == "/ROOT/daily[]/_obj") {
    dailyItemCounter++;
  }  
  currentKey = "";
  currentParent= currentParent.substring(0, currentParent.lastIndexOf(PATH_SEPERATOR));
}

void OpenWeatherMapOneCall::endDocument()
{
}

void OpenWeatherMapOneCall::startArray()
{
  weatherItemCounter = 0;
  
  currentParent += PATH_SEPERATOR + currentKey + "[]";
  currentKey = "";
}

String OpenWeatherMapOneCall::getMeteoconIcon(String icon)
{
  // clear sky
  // 01d
  if (icon == "01d") {
    return "B";
  }
  // 01n
  if (icon == "01n") {
    return "C";
  }
  // few clouds
  // 02d
  if (icon == "02d") {
    return "H";
  }
  // 02n
  if (icon == "02n") {
    return "4";
  }
  // scattered clouds
  // 03d
  if (icon == "03d") {
    return "N";
  }
  // 03n
  if (icon == "03n") {
    return "5";
  }
  // broken clouds
  // 04d
  if (icon == "04d") {
    return "Y";
  }
  // 04n
  if (icon == "04n") {
    return "%";
  }
  // shower rain
  // 09d
  if (icon == "09d") {
    return "R";
  }
  // 09n
  if (icon == "09n") {
    return "8";
  }
  // rain
  // 10d
  if (icon == "10d") {
    return "Q";
  }
  // 10n
  if (icon == "10n") {   
    return "7";
  }
  // thunderstorm
  // 11d
  if (icon == "11d") {
    return "P";
  }
  // 11n
  if (icon == "11n") {
    return "6";
  }
  // snow
  // 13d
  if (icon == "13d") {
    return "W";
  }
  // 13n
  if (icon == "13n") {
    return "#";
  }
  // mist
  // 50d
  if (icon == "50d") {
    return "M";
  }
  // 50n
  if (icon == "50n") {
    return "M";
  }
  // Nothing matched: N/A
  return ")";
}
