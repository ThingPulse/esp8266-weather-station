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
*
*   CLIENT PURPOSE:
*   This client will query the Met Office 3-hourly Datapoint API, which returns forecasts for a specific location
*   for the upcoming 5 days split into 3-hour sections:
*   http://www.metoffice.gov.uk/datapoint/product/uk-3hourly-site-specific-forecast
*
*   This client will create an array of a subset of the forecast details (see value() for which ones) and
*   provides getters for each in a hardcoded way for the current period and the subsequent three. It can be
*   expanded to cover further periods if required, hopefully the comments within the client details how.
*
*   USAGE INSTRUCTIONS:
*   You will need an API key to use these services, available at http://www.metoffice.gov.uk/datapoint/api
*   You will also need to determine your location ID. To get that you need to hit this url:
*   http://datapoint.metoffice.gov.uk/public/data/val/wxfcs/all/json/sitelist?key=[YOUR_API_KEY_HERE]
*   and search for your nearest location. If you can't find the location you are after you can hit the main
*   site and use their front-end search to find the closest location they have, then get the code for that
*   from the api response above.
*/

#include <ESPWiFi.h>
#include "MetOfficeClient.h"

MetOfficeClient::MetOfficeClient() {
}

void MetOfficeClient::updateConditions(String currentForecastPeriod, int location, String apiKey) {
  /* The Met Office 3-hourly API returns forecasts for 40 consecutive timeframes, and doesn't necessarily
   * start with the current one. The only way to identify periods is via the $ value within the specific forecast,
   * which holds the period defined in number of minutes past midnight. This means we need to know what periods
   * we are interested in in those terms, and then capture the information just for those periods. */

   // TODO - Need to find some way to handle periods that cross into the new day within the API response

  currentForecast = currentForecastPeriod;
  // If we wanted more (or less) forecasts we would need to add (or remove) them here
  secondForecast = String(currentForecast.toInt() + 180);
  thirdForecast = String(currentForecast.toInt() + 360);
  fourthForecast = String(currentForecast.toInt() + 540);
  String url = "/public/data/val/wxfcs/all/json/";
  url += location;
  url += "?res=3hourly&key=";
  url += apiKey;
  doUpdate(url);
}

void MetOfficeClient::doUpdate(String url) {
  JsonStreamingParser parser;
  parser.setListener(this);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("datapoint.metoffice.gov.uk", httpPort)) {
    Serial.println("connection failed");
    return;
  }
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
       "Host: datapoint.metoffice.gov.uk\r\n" +
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

  client.setNoDelay(false);
  while (client.connected() || client.available()) {
    if (client.available()) {
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
}

void MetOfficeClient::whitespace(char c) {
  Serial.println("whitespace");
}

void MetOfficeClient::startDocument() {
  Serial.println("start document");
}

void MetOfficeClient::key(String key) {
  currentKey = String(key);
}

void MetOfficeClient::value(String value) {
  /* Because forecast period is AFTER all the values we want we have to capture all forecast values we
   * are interested in up front on parsing, then only write them to an array we will use in the value
   * getters if they are for a period that we need */
  if (currentKey == "D") {
    windDirection = value;
  }
  if (currentKey == "F") {
    feelsLikeTemp = value;
  }
  if (currentKey == "G") {
    windGust = value;
  }
  if (currentKey == "Pp") {
    precipProb = value;
  }
  if (currentKey == "S") {
    windSpeed = value;
  }
  if (currentKey == "T") {
    temperature = value;
  }
  if (currentKey == "W") {
    weatherType = value;
  }
  if (currentKey == "U") {
    maxUvIndex = value;
  }
  /* Creating an array of all the values we want for the required forecast periods only. Would need to change
   * the upper value of count in the if statement below if we wanted to capture more periods (or lower it if
   * we want to capture less) */
  if (currentKey == "$" && (value == currentForecast || value == secondForecast || value == thirdForecast
          || value == fourthForecast) && count < 36) {
    forecastArray[count] = value;
    count++;
    forecastArray[count] = windDirection;
    count++;
    forecastArray[count] += feelsLikeTemp;
    count++;
    forecastArray[count] += windGust;
    count++;
    forecastArray[count] += precipProb;
    count++;
    forecastArray[count] += windSpeed;
    count++;
    forecastArray[count] += temperature;
    count++;
    forecastArray[count] += weatherType;
    count++;
    forecastArray[count] += maxUvIndex;
    count++;
  }

  if (currentKey == "name") {
    locationName = value;
  }

  if (currentKey == "dataDate") {
    currentDate = value;
  }
}

void MetOfficeClient::endArray() {
}

void MetOfficeClient::startObject() {
  currentParent = currentKey;
}

void MetOfficeClient::endObject() {
  currentParent = "";
}

void MetOfficeClient::endDocument() {
}

void MetOfficeClient::startArray() {
}

String MetOfficeClient::getLocationName() {
  return locationName;
}

String MetOfficeClient::getCurrentDate() {
  return currentDate;
}

String MetOfficeClient::getCurrentForecastTime() {
  // Simplest thing here is to convert it to 24 hour clock value
  int currentHour = (forecastArray[0].toInt() / 60);
  String currentForecastTime = "";
  if (currentHour < 10) {
    currentForecastTime = "0" + String(currentHour) + ":00";
  } else {
    currentForecastTime = String(currentHour) + ":00";
  }
  return currentForecastTime;
}

String MetOfficeClient::getCurrentWindDirection() {
  String currentWindDirection = forecastArray[1];
  return currentWindDirection;
}

String MetOfficeClient::getCurrentFeelsLikeTemp() {
  String currentFeelsLikeTemp = forecastArray[2];
  return currentFeelsLikeTemp;
}

String MetOfficeClient::getCurrentWindGust() {
  String currentWindGust = forecastArray[3];
  return currentWindGust;
}

String MetOfficeClient::getCurrentPrecipProb() {
  String currentPrecipProb = forecastArray[4];
  return currentPrecipProb;
}

String MetOfficeClient::getCurrentWindSpeed() {
  String currentWindSpeed = forecastArray[5];
  return currentWindSpeed;
}

String MetOfficeClient::getCurrentTemperature() {
  String currentTemperature = forecastArray[6];
  return currentTemperature;
}

String MetOfficeClient::getCurrentWeatherType() {
  String currentWeatherType = forecastArray[7];
  return currentWeatherType;
}

String MetOfficeClient::getCurrentMaxUvIndex() {
  String currentMaxUvIndex = forecastArray[8];
  return currentMaxUvIndex;
}

String MetOfficeClient::getSecondForecastTime() {
  // Simplest thing here is to convert it to 24 hour clock value
  int secondHour = (forecastArray[9].toInt() / 60);
  String secondForecastTime = "";
  if (secondHour < 10) {
    secondForecastTime = "0" + String(secondHour) + ":00";
  } else {
    secondForecastTime = String(secondHour) + ":00";
  }
  return secondForecastTime;
}

String MetOfficeClient::getSecondWindDirection() {
  String secondWindDirection = forecastArray[10];
  return secondWindDirection;
}

String MetOfficeClient::getSecondFeelsLikeTemp() {
  String secondFeelsLikeTemp = forecastArray[11];
  return secondFeelsLikeTemp;
}

String MetOfficeClient::getSecondWindGust() {
  String secondWindGust = forecastArray[12];
  return secondWindGust;
}

String MetOfficeClient::getSecondPrecipProb() {
  String secondPrecipProb = forecastArray[13];
  return secondPrecipProb;
}

String MetOfficeClient::getSecondWindSpeed() {
  String secondWindSpeed = forecastArray[14];
  return secondWindSpeed;
}

String MetOfficeClient::getSecondTemperature() {
  String secondTemperature = forecastArray[15];
  return secondTemperature;
}

String MetOfficeClient::getSecondWeatherType() {
  String secondWeatherType = forecastArray[16];
  return secondWeatherType;
}

String MetOfficeClient::getSecondMaxUvIndex() {
  String secondMaxUvIndex = forecastArray[17];
  return secondMaxUvIndex;
}

String MetOfficeClient::getThirdForecastTime() {
  // Simplest thing here is to convert it to 24 hour clock value
  int thirdHour = (forecastArray[18].toInt() / 60);
  String thirdForecastTime = "";
  if (thirdHour < 10) {
    thirdForecastTime = "0" + String(thirdHour) + ":00";
  } else {
    thirdForecastTime = String(thirdHour) + ":00";
  }
  return thirdForecastTime;
}

String MetOfficeClient::getThirdWindDirection() {
  String thirdWindDirection = forecastArray[19];
  return thirdWindDirection;
}

String MetOfficeClient::getThirdFeelsLikeTemp() {
  String thirdFeelsLikeTemp = forecastArray[20];
  return thirdFeelsLikeTemp;
}

String MetOfficeClient::getThirdWindGust() {
  String thirdWindGust = forecastArray[21];
  return thirdWindGust;
}

String MetOfficeClient::getThirdPrecipProb() {
  String thirdPrecipProb = forecastArray[22];
  return thirdPrecipProb;
}

String MetOfficeClient::getThirdWindSpeed() {
  String thirdWindSpeed = forecastArray[23];
  return thirdWindSpeed;
}

String MetOfficeClient::getThirdTemperature() {
  String thirdTemperature = forecastArray[24];
  return thirdTemperature;
}

String MetOfficeClient::getThirdWeatherType() {
  String thirdWeatherType = forecastArray[25];
  return thirdWeatherType;
}

String MetOfficeClient::getThirdMaxUvIndex() {
  String thirdMaxUvIndex = forecastArray[26];
  return thirdMaxUvIndex;
}

String MetOfficeClient::getFourthForecastTime() {
  // Simplest thing here is to convert it to 24 hour clock value
  int fourthHour = (forecastArray[27].toInt() / 60);
  String fourthForecastTime = "";
  if (fourthHour < 10) {
    fourthForecastTime = "0" + String(fourthHour) + ":00";
  } else {
    fourthForecastTime = String(fourthHour) + ":00";
  }
  return fourthForecastTime;
}

String MetOfficeClient::getFourthWindDirection() {
  String fourthWindDirection = forecastArray[28];
  return fourthWindDirection;
}

String MetOfficeClient::getFourthFeelsLikeTemp() {
  String fourthFeelsLikeTemp = forecastArray[29];
  return fourthFeelsLikeTemp;
}

String MetOfficeClient::getFourthWindGust() {
  String fourthWindGust = forecastArray[30];
  return fourthWindGust;
}

String MetOfficeClient::getFourthPrecipProb() {
  String fourthPrecipProb = forecastArray[31];
  return fourthPrecipProb;
}

String MetOfficeClient::getFourthWindSpeed() {
  String fourthWindSpeed = forecastArray[32];
  return fourthWindSpeed;
}

String MetOfficeClient::getFourthTemperature() {
  String fourthTemperature = forecastArray[33];
  return fourthTemperature;
}

String MetOfficeClient::getFourthWeatherType() {
  String fourthWeatherType = forecastArray[34];
  return fourthWeatherType;
}

String MetOfficeClient::getFourthMaxUvIndex() {
  String fourthMaxUvIndex = forecastArray[35];
  return fourthMaxUvIndex;
}

String MetOfficeClient::getWeatherIconName(String weatherType) {
  /* Need to stick to <= 8 chars as these values are the names for the bitmap icons
   * Details taken from http://www.metoffice.gov.uk/datapoint/support/documentation/code-definitions
   * Currently mapping against the black backgrounded wunderground icons taken from squix78's blog site
   * as per https://github.com/squix78/esp8266-weather-station-color/blob/master/esp8266-weather-station-color.ino#L192
   * Could remap against Met Office versions if wanted to use white background on display
   * http://www.metoffice.gov.uk/guide/weather/symbols#weathersymbols */
  if (weatherType == "0") return "d_sun";     // Clear night
  if (weatherType == "1") return "d_sun";     // Sunny day
  if (weatherType == "2") return "d_prtcld";  // Partly cloudy (night)
  if (weatherType == "3") return "d_prtcld";  // Partly cloudy (day)
  if (weatherType == "4") return "no_data";   // Marked not used by API docs
  if (weatherType == "5") return "fog";       // Mist
  if (weatherType == "6") return "fog";       // Fog
  if (weatherType == "7") return "cloudy";    // Cloudy
  if (weatherType == "8") return "cloudy";    // Overcast
  if (weatherType == "9") return "d_lrshwr";  // Light rain shower (night)
  if (weatherType == "10") return "d_lrshwr"; // Light rain shower (day)
  if (weatherType == "11") return "d_lrshwr"; // Drizzle
  if (weatherType == "12") return "d_lrshwr"; // Light rain
  if (weatherType == "13") return "hyrain";   // Heavy rain shower (night)
  if (weatherType == "14") return "hyrain";   // Heavy rain shower (day)
  if (weatherType == "15") return "hyrain";   // Heavy rain
  if (weatherType == "16") return "d_slshwr"; // Sleet shower (night)
  if (weatherType == "17") return "d_slshwr"; // Sleet shower (day)
  if (weatherType == "18") return "sleet";    // Sleet
  if (weatherType == "19") return "d_hlshwr"; // Hail shower (night)
  if (weatherType == "20") return "d_hlshwr"; // Hail shower (day)
  if (weatherType == "21") return "hail";     // Hail
  if (weatherType == "22") return "d_lsshwr"; // Light snow shower (night)
  if (weatherType == "23") return "d_lsshwr"; // Light snow shower (day)
  if (weatherType == "24") return "d_lsshwr"; // Light snow
  if (weatherType == "25") return "hysnow";   // Heavy snow shower (night)
  if (weatherType == "26") return "hysnow";   // Heavy snow shower (day)
  if (weatherType == "27") return "hysnow";   // Heavy snow
  if (weatherType == "28") return "d_thshwr"; // Thunder shower (night)
  if (weatherType == "29") return "d_thshwr"; // Thunder shower (night)
  if (weatherType == "30") return "thunder";  // Thunder
  return "no_data";// Something unexpected has happened!
}
