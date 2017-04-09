#include "AdsbExchangeClient.h"


AdsbExchangeClient::AdsbExchangeClient() {

}

void AdsbExchangeClient::updateVisibleAircraft(String searchQuery) {
  JsonStreamingParser parser;
  parser.setListener(this);
  WiFiClient client;

  // http://global.adsbexchange.com/VirtualRadar/AircraftList.json?lat=47.437691&lng=8.568854&fDstL=0&fDstU=20&fAltL=0&fAltU=5000
  const char host[] = "global.adsbexchange.com";
  String url = "/VirtualRadar/AircraftList.json?" + searchQuery;

  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }


  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

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
  endDocument();
}

String AdsbExchangeClient::getFrom() {
  if (from[CURRENT].length() >=4) {
      int firstComma = from[CURRENT].indexOf(",");
      return from[CURRENT].substring(5, firstComma);
  }
  return "";
}
String AdsbExchangeClient::getFromIcao() {
  if (from[CURRENT].length() >=4) {
      return from[CURRENT].substring(0,4);
  }
  return "";
}
String AdsbExchangeClient::getTo() {
  if (to[CURRENT].length() >=4) {
      int firstComma = to[CURRENT].indexOf(",");
      return to[CURRENT].substring(5, firstComma);
  }
  return "";
}

String AdsbExchangeClient::getToIcao() {
  if (to[CURRENT].length() >=4) {
      return to[CURRENT].substring(0,4);
  }
  return "";
}
String AdsbExchangeClient::getAltitude(){
  return altitude[CURRENT];
}
double AdsbExchangeClient::getDistance() {
  return distance[CURRENT];

}
String AdsbExchangeClient::getAircraftType() {
  return aircraftType[CURRENT];

}
String AdsbExchangeClient::getOperatorCode() {
  return operatorCode[CURRENT];
}

double AdsbExchangeClient::getHeading() {
  return heading[CURRENT];
}

void AdsbExchangeClient::whitespace(char c) {

}

void AdsbExchangeClient::startDocument() {
  counter = 0;
  currentMinDistance = 1000.0;
}

void AdsbExchangeClient::key(String key) {
  currentKey = key;
}

void AdsbExchangeClient::value(String value) {
  /*String from = "";
  String to = "";
  String altitude = "";
  String aircraftType = "";
  String currentKey = "";
  String operator = "";


 "Type": "A319",
 "Mdl": "Airbus A319 112",

 "From": "LSZH Z\u00c3\u00bcrich, Zurich, Switzerland",
 "To": "LEMD Madrid Barajas, Spain",
 "Op": "Swiss International Air Lines",
 "OpIcao": "SWR",
 "Dst": 6.23,
 "Year": "1996"
 */
  if (currentKey == "Id") {
    counter++;
  } else if (currentKey == "From") {
    from[TEMP] = value;
  } else if (currentKey == "To") {
    to[TEMP] = value;
  } else if (currentKey == "OpIcao") {
    operatorCode[TEMP] = value;
  } else if (currentKey == "Dst") {
    distance[TEMP] = value.toFloat();
  } else if (currentKey == "Mdl") {
    aircraftType[TEMP] = value;
  } else if (currentKey == "Trak") {
    heading[TEMP] = value.toFloat();
  } else if (currentKey == "Alt") {
    altitude[TEMP] = value;
  } else if (currentKey == "Trt") {
    if (distance[TEMP] < currentMinDistance) {
      currentMinDistance = distance[TEMP];
      Serial.println("Found a closer aircraft");
      from[CURRENT] = from[TEMP];
      to[CURRENT] = to[TEMP];
      altitude[CURRENT] = altitude[TEMP];
      distance[CURRENT] = distance[TEMP];
      aircraftType[CURRENT] = aircraftType[TEMP];
      operatorCode[CURRENT] = operatorCode[TEMP];
      heading[CURRENT] = heading[TEMP];
    }
  }
  Serial.println(currentKey + "=" + value);
}

int AdsbExchangeClient::getNumberOfVisibleAircrafts() {
  return counter;
}

void AdsbExchangeClient::endArray() {

}

void AdsbExchangeClient::endObject() {

}

void AdsbExchangeClient::endDocument() {
  Serial.println("Flights: " + String(counter));
  if (counter == 0 && lastSightingMillis < millis() - MAX_AGE_MILLIS) {
    for (int i = 0; i < 2; i++) {
      from[i] = "";
      to[i] = "";
      altitude[i] = "";
      distance[i] = 1000.0;
      aircraftType[i] = "";
      operatorCode[i] = "";
      heading[i] = 0.0;
    }
  } else if (counter > 0) {
    lastSightingMillis = millis();
  }
}

boolean AdsbExchangeClient::isAircraftVisible() {
  return counter > 0 || lastSightingMillis > millis() - MAX_AGE_MILLIS;
}

void AdsbExchangeClient::startArray() {

}

void AdsbExchangeClient::startObject() {

}
