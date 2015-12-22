#pragma once

#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48

class NTPClient {
  private:
    WiFiUDP       _udp;

    const char*   _poolServerName = "time.nist.gov"; // Default time server
    int           _port           = 1337;
    int           _timeOffset;

    unsigned int  _updateInterval = 60000;  // In ms

    unsigned long _currentEpoc;             // In s
    unsigned long _lastUpdate     = 0;      // In ms

    byte          _packetBuffer[NTP_PACKET_SIZE];

    void          sendNTPPacket(IPAddress _timeServerIP);

  public:
    NTPClient(int timeOffset);
    NTPClient(const char* poolServerName);
    NTPClient(const char* poolServerName, int timeOffset);
    NTPClient(const char* poolServerName, int timeOffset, int updateInterval);

    void begin();
    void update();
    void forceUpdate();

    String getHours();
    String getMinutes();
    String getSeconds();
    String getFormattedTime();

    unsigned long getRawTime();
};
