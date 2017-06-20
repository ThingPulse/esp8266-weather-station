#include "ThingspeakClient.h"


ThingspeakClient::ThingspeakClient() {

}

void ThingspeakClient::getLastChannelItem(String channelId, String readApiKey) {
  JsonStreamingParser parser;
  parser.setListener(this);
  WiFiClient client;

  // http://api.thingspeak.com/channels/CHANNEL_ID/feeds.json?results=2&api_key=API_KEY
  const char host[] = "api.thingspeak.com";
  String url = "/channels/" + channelId +"/feeds.json?results=1&api_key=" + readApiKey;

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
}

void ThingspeakClient::whitespace(char c) {

}

void ThingspeakClient::startDocument() {

}

void ThingspeakClient::key(String key) {
  if (key == "channel") {
    isHeader = true;
  } else if (key == "feeds") {
    isHeader = false;
  }
  currentKey = key;
}

void ThingspeakClient::value(String value) {
    //Serial.println(currentKey +": " + value);

      for (int i = 1; i < 9; i++) {
        String fieldKey = "field" + String(i);

        if (currentKey == fieldKey) {
          if (isHeader) {
            fieldLabels[i-1] = value;
          } else {
            lastFields[i-1] = value;
            Serial.println(fieldKey + ": " + value);
          }

        }
      }


}


String ThingspeakClient::getFieldLabel(int index) {
  return fieldLabels[index];
}

String ThingspeakClient::getFieldValue(int index) {
  return lastFields[index];
}

String ThingspeakClient::getCreatedAt() {
  return createdAt;
}

void ThingspeakClient::endArray() {

}

void ThingspeakClient::endObject() {

}

void ThingspeakClient::endDocument() {

}

void ThingspeakClient::startArray() {

}

void ThingspeakClient::startObject() {

}
