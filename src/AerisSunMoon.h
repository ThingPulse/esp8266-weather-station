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

#pragma once
#include <Arduino.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>

typedef struct AerisSunMoonData {
  uint64_t sunRise; // "rise":1493291184,
  String sunRiseISO; // "riseISO":"2017-04-27T06:06:24-05:00",
  uint64_t sunSet; // "set":1493342079,
  String sunSetISO; //"setISO":"2017-04-27T20:14:39-05:00",
  uint64_t sunTransit; // "transit":1493316631,
  String sunTransitISO; // "transitISO":"2017-04-27T13:10:31-05:00",
  boolean midnightSun; // "midnightSun":false,
  boolean polarNight; // "polarNight":false,
  uint64_t moonRise; //"rise":1493295480,
  String moonRiseISO; // "riseISO":"2017-04-27T07:18:00-05:00",
  uint64_t moonSet; // "set":1493347800,
  String moonSetISO; // "setISO":"2017-04-27T21:50:00-05:00",
  uint64_t moonTransit; // "transit":1493321340,
  String moonTransitISO; // "transitISO":"2017-04-27T14:29:00-05:00",
  uint64_t moonUnderfoot; // "underfoot":1493276400,
  String moonUnderfootISO; // "underfootISO":"2017-04-27T02:00:00-05:00",
  float moonPhase; // "phase":0.0516,
  String moonPhaseName; // "name":"waxing crescent",
  uint8_t moonIllum; // "illum":3,
  float moonAge; // "age":1.52,
  float moonAngle; // "angle":0.55
} AerisSunMoonData;

class AerisSunMoon: public JsonListener {
  private:
    const String host = "api.aerisapi.com";
    const uint8_t port = 80;
    boolean isMetric = true;
    String currentKey;
    String currentParent;
    AerisSunMoonData *sunMoonData;


   void doUpdate(AerisSunMoonData *sunMoonData, String url);

  public:
    AerisSunMoon();
    void updateSunMoon(AerisSunMoonData *sunMoonData, String clientId, String clientKey, String location);

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
