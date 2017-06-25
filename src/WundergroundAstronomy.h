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

#pragma once

#include <JsonListener.h>
#include <JsonStreamingParser.h>

typedef struct WGAstronomy {
  String moonPctIlum;
  String moonAge;
  String moonPhase;
  String sunriseTime;
  String sunsetTime;
  String moonriseTime;
  String moonsetTime;
} WGAstronomy;

class WundergroundAstronomy: public JsonListener {
  private:
    WGAstronomy *astronomy;
    String currentKey;
    String currentParent = "";
    String moonPctIlum;  // not used
    String moonAge;      // make this a long?
    String moonPhase;
    String sunriseTime;
    String sunsetTime;
    String moonriseTime;
    String moonsetTime;

    boolean usePM;
    boolean isPM;

    void doUpdate(WGAstronomy *astronomy, String url);


  public:
    WundergroundAstronomy(boolean usePM);
    void updateAstronomy(WGAstronomy *astronomy, String apiKey, String language, String country, String city);
    void updateAstronomyPWS(WGAstronomy *astronomy, String apiKey, String language, String pws);

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
