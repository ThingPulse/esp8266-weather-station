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
*   Based on esp8266-weather-station-color created by Daniel Eichhorn:
*   https://github.com/squix78/esp8266-weather-station-color
*
*   This demo uses a tft screen with a built in sd card reader, with both connected
*   to an esp-12e via SPI.
*   It displays the current and next 3 period forecasts on the screen, using bmp images
*   stored on the SD card.
*   You will need to get an API key and location id from the Met Office, as described in the
*   client, and add them to settings.h together with wifi details and an OTA password if you want
*   to use that.
*
*/

#include <Arduino.h>
#include <SD.h>
#include <ESPWiFi.h>
#include <ESPHTTPClient.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
// Below required to allow access to the functions required to implement light_sleep mode on esp8266
extern "C" {
  #include "user_interface.h"
}
// Additional UI functions
#define FS_NO_GLOBALS //allow spiffs to coexist with SD card - https://github.com/esp8266/Arduino/issues/1524
#include <GfxUi.h>
// Fonts created by http://oleddisplay.squix.ch/
#include <LiberationSerifBold_14.h>
#include <LiberationSerifBold_20.h>
#include <LiberationSerifBold_36.h>

// Courtesy of https://github.com/squix78
#include <JsonListener.h>
#include <MetOfficeClient.h>
#include <TimeClient.h>

#include <settings.h>

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
GfxUi ui = GfxUi(&tft);

MetOfficeClient metOffice;
TimeClient timeClient(UTC_OFFSET);

// Declaring prototypes
void wifiSetup();
void otaSetup();
void drawBmpFromSdCard(char *filename, int16_t x, int16_t y);
uint16_t read16(File &f);
uint32_t read32(File &f);
void updateDataWithProgress();
void updateDataSilently();
void drawForecastDetails();
void drawProgress(uint8_t percentage, String text);
void drawSeparator(uint16_t y);
void drawLocationDetails();
void drawLastUpdateTime();
void drawCurrentForecast();
void drawSecondForecast();
void drawThirdForecast();
void drawFourthForecast();
void getCurrentForecastPeriod();

String currentForecastPeriod = "";  // Need to pick up next three forecasts from starting point
String forecastArray[36] = { };   // String array to hold the segmented forecast data for the required 3 periods
unsigned long upTime = 0;     // Used to determine if it is time to enter light sleep mode indefinitely, to awake via external reset button
unsigned long lastUpdateTime = 0;  // Used to determine if should update data, rather than just sticking a big delay in loop()...

void setup() {
  Serial.begin(115200);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);

  // Connecting the LED pin to the ESP8266 rather than direct to Vcc allows it to be turned off
  // when in deep sleep
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  setDeviceUp();

  updateDataWithProgress();
  drawForecastDetails();
}

void loop() {
  ArduinoOTA.handle();
  upTime = millis(); // Get current upTime so can see if time to refresh data
  if ((upTime - lastUpdateTime)/1000 >= UPDATE_INTERVAL_SECS) {
    updateDataSilently();
    drawForecastDetails();
  }
  if ((upTime/1000) >= TIME_BEFORE_SLEEP_SECS) {
    Serial.println("Shutting screen off");
    tft.fillScreen(ILI9341_BLACK);
    digitalWrite(TFT_LED, LOW);
    delay(1000);
    Serial.println("Going to sleep");
    wifi_fpm_open(); // Enables force sleep
    wifi_fpm_do_sleep(0xFFFFFFF); // Sleep for longest possible time
  }
}

void setDeviceUp() {
  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(&LiberationSerifBold_14);
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  drawProgress(25, "Connecting to WiFi...");
  wifiSetup();
  drawProgress(50, "Starting OTA...");
  otaSetup();
  drawProgress(75, "Initialising SD card...");
  SD.begin(SD_CS);
  drawProgress(100, "Setup complete!");
  delay(100);
}

void wifiSetup() {
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise by default it,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void otaSetup() {
  // Set up OTA ability
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void drawBmpFromSdCard(String filename, int16_t x, int16_t y) {
  // Couldn't find an easy way to add this function to GfxUi.cpp, kept getting compile errors on libraries
  // so something must have been clashing
  // This is taken from https://github.com/adafruit/Adafruit_ILI9341/blob/master/examples/spitftbitmap/spitftbitmap.ino

  File bmpFile;
  int bmpWidth, bmpHeight;  // W+H in pixels
  uint8_t bmpDepth;   // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;  // Start of image data in file
  uint32_t rowSize;   // Not always = bmpWidth; may have padding
  uint8_t sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean goodBmp = false;  // Set to true on valid header parse
  boolean flip  = true;   // BMP is stored bottom-to-top
  int w, h, row, col, x2, y2, bx1, by1;
  uint8_t r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip  = false;
        }

        // Crop area to be loaded
        x2 = x + bmpWidth  - 1;// Lower-right corner
        y2 = y + bmpHeight - 1;
        if((x2 >= 0) && (y2 >= 0)) { // On screen?
          w = bmpWidth; // Width/height of section to load/display
          h = bmpHeight;
          bx1 = by1 = 0; // UL coordinate in BMP file
          if(x < 0) { // Clip left
            bx1 = -x;
            x   = 0;
            w   = x2 + 1;
          }
          if(y < 0) { // Clip top
            by1 = -y;
            y   = 0;
            h   = y2 + 1;
          }
          if(x2 >= tft.width()) w = tft.width()  - x; // Clip right
          if(y2 >= tft.height()) h = tft.height() - y; // Clip bottom

          // Set TFT address window to clipped image bounds
          tft.startWrite(); // Requires start/end transaction now
          tft.setAddrWindow(x, y, w, h);

          for (row=0; row<h; row++) { // For each scanline...

            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (bmpHeight - 1 - (row + by1)) * rowSize;
            else // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + (row + by1) * rowSize;
            pos += bx1 * 3; // Factor in starting column (bx1)
            if(bmpFile.position() != pos) { // Need seek?
              tft.endWrite(); // End TFT transaction
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer); // Force buffer reload
              tft.startWrite(); // Start new TFT transaction
            }
            for (col=0; col<w; col++) { // For each pixel...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer)) { // Indeed
                tft.endWrite(); // End TFT transaction
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0; // Set index to beginning
                tft.startWrite(); // Start new TFT transaction
              }
              // Convert pixel from BMP to TFT format, push to display
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              tft.writePixel(tft.color565(r,g,b));
            } // end pixel
          } // end scanline
          tft.endWrite(); // End last TFT transaction
        } // end onscreen
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void updateDataWithProgress() {
  Serial.println("Updating data with progress");
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(&LiberationSerifBold_14);
  drawProgress(33, "Updating time...");
  timeClient.updateTime();
  drawProgress(66, "Updating forecasts...");
  getCurrentForecastPeriod();
  metOffice.updateConditions(currentForecastPeriod, LOC_CODE, API_KEY);
  drawProgress(100, "Updated!");
  lastUpdateTime = millis();
}

void updateDataSilently() {
  Serial.println("Updating data silently");
  timeClient.updateTime();
  getCurrentForecastPeriod();
  metOffice.updateConditions(currentForecastPeriod, LOC_CODE, API_KEY);
  lastUpdateTime = millis();
}

void drawForecastDetails() {
  tft.fillScreen(ILI9341_BLACK);
  drawLocationDetails();
  drawSeparator(50);
  drawCurrentForecast();
  drawSecondForecast();
  drawThirdForecast();
  drawFourthForecast();
  drawLastUpdateTime();
}

void drawProgress(uint8_t percentage, String text) {
  ui.setTextAlignment(CENTER);
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  tft.fillRect(0, 140, 240, 45, ILI9341_BLACK);
  ui.drawString(120, 155, text);
  ui.drawProgressBar(10, 165, 240 - 20, 15, percentage, ILI9341_WHITE, ILI9341_BLUE);
}

void drawSeparator(uint16_t y) {
  tft.fillRect(10, y, 220, 3, ILI9341_WHITE);
}

void drawLocationDetails() {
  ui.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  tft.setFont(&LiberationSerifBold_36);
  String locationName = metOffice.getLocationName();
  ui.drawString(120, 35, locationName);
}

void drawLastUpdateTime() {
  ui.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  drawSeparator(295);
  ui.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  tft.setFont(&LiberationSerifBold_14);
  String currentDateUnformatted = metOffice.getCurrentDate();
  String currentDay = currentDateUnformatted.substring(8, 10);
  String currentMonth = currentDateUnformatted.substring(5, 7);
  String currentYear = currentDateUnformatted.substring(2, 4);
  String currentDateFormatted = currentDay + "/" + currentMonth + "/" + currentYear;
  String currentTime = timeClient.getFormattedTime();
  ui.drawString(120, 315, "Updated: " + currentDateFormatted + " : " + currentTime);
}

void getCurrentForecastPeriod() {
  String currentHour = timeClient.getHours();
  int currentMinutesSinceMidnight = currentHour.toInt() * 60;
  int currentPeriodNumber = floor(currentMinutesSinceMidnight / 180); // Calculate which forecast period currently in
  currentForecastPeriod = String(currentPeriodNumber * 180);
}

void drawCurrentForecast() {
  String forecastTime = metOffice.getCurrentForecastTime();
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  tft.setFont(&LiberationSerifBold_14);
  ui.drawString(37, 75, forecastTime);
  String weatherType = metOffice.getCurrentWeatherType();
  String weatherIcon = metOffice.getWeatherIconName(weatherType);
  drawBmpFromSdCard(weatherIcon + ".bmp", 15, 80);
  tft.setFont(&LiberationSerifBold_36);
  ui.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  String temperature = metOffice.getCurrentTemperature();
  tft.setFont(&LiberationSerifBold_36);
  ui.drawString(33, 157, temperature);
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  String feelsLikeTemp = metOffice.getCurrentFeelsLikeTemp();
  ui.drawString(33, 196, feelsLikeTemp);
  tft.setFont(&LiberationSerifBold_20);
  ui.setTextColor(ILI9341_BLUE, ILI9341_BLACK);
  String precipProb = metOffice.getCurrentPrecipProb();
  ui.drawString(33, 222, precipProb + "%");
  String maxUvIndex = metOffice.getCurrentMaxUvIndex();
  String uvIcon = "";
  if (maxUvIndex.toInt() <= 2) { uvIcon = "uvGreen";}
  if (3 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 5) { uvIcon = "uvYellow";}
  if (6 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 7) { uvIcon = "uvOrange";}
  if (8 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 10) { uvIcon = "uvRed";}
  if (maxUvIndex.toInt() > 10) { uvIcon = "uvPurple";}
  drawBmpFromSdCard(uvIcon + ".bmp", 15, 227);
  String windSpeed = metOffice.getCurrentWindSpeed();
  String windGust = metOffice.getCurrentWindGust();
  tft.setFont(&LiberationSerifBold_14);
  ui.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  ui.drawString(33, 287, windSpeed + "/" + windGust);
}

void drawSecondForecast() {
  String forecastTime = metOffice.getSecondForecastTime();
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  tft.setFont(&LiberationSerifBold_14);
  ui.drawString(91, 75, forecastTime);
  String weatherType = metOffice.getSecondWeatherType();
  String weatherIcon = metOffice.getWeatherIconName(weatherType);
  drawBmpFromSdCard(weatherIcon + ".bmp", 69, 80);
  tft.setFont(&LiberationSerifBold_36);
  ui.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  String temperature = metOffice.getSecondTemperature();
  ui.drawString(89, 157, temperature);
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  String feelsLikeTemp = metOffice.getSecondFeelsLikeTemp();
  ui.drawString(89, 196, feelsLikeTemp);
  ui.setTextColor(ILI9341_BLUE, ILI9341_BLACK);
  String precipProb = metOffice.getSecondPrecipProb();
  tft.setFont(&LiberationSerifBold_20);
  ui.drawString(89, 222, precipProb + "%");
  String maxUvIndex = metOffice.getSecondMaxUvIndex();
  String uvIcon = "";
  if (maxUvIndex.toInt() <= 2) { uvIcon = "uvGreen";}
  if (3 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 5) { uvIcon = "uvYellow";}
  if (6 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 7) { uvIcon = "uvOrange";}
  if (8 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 10) { uvIcon = "uvRed";}
  if (maxUvIndex.toInt() > 10) { uvIcon = "uvPurple";}
  drawBmpFromSdCard(uvIcon + ".bmp", 69, 227);
  String windSpeed = metOffice.getSecondWindSpeed();
  String windGust = metOffice.getSecondWindGust();
  ui.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setFont(&LiberationSerifBold_14);
  ui.drawString(89, 287, windSpeed + "/" + windGust);
}

void drawThirdForecast() {
  String forecastTime = metOffice.getThirdForecastTime();
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  tft.setFont(&LiberationSerifBold_14);
  ui.drawString(147, 75, forecastTime);
  String weatherType = metOffice.getThirdWeatherType();
  String weatherIcon = metOffice.getWeatherIconName(weatherType);
  drawBmpFromSdCard(weatherIcon + ".bmp", 125, 80);
  tft.setFont(&LiberationSerifBold_36);
  ui.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  String temperature = metOffice.getThirdTemperature();
  ui.drawString(145, 157, temperature);
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  String feelsLikeTemp = metOffice.getThirdFeelsLikeTemp();
  ui.drawString(145, 196, feelsLikeTemp);
  ui.setTextColor(ILI9341_BLUE, ILI9341_BLACK);
  String precipProb = metOffice.getThirdPrecipProb();
  tft.setFont(&LiberationSerifBold_20);
  ui.drawString(145, 222, precipProb + "%");
  String maxUvIndex = metOffice.getThirdMaxUvIndex();
  String uvIcon = "";
  if (maxUvIndex.toInt() <= 2) { uvIcon = "uvGreen";}
  if (3 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 5) { uvIcon = "uvYellow";}
  if (6 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 7) { uvIcon = "uvOrange";}
  if (8 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 10) { uvIcon = "uvRed";}
  if (maxUvIndex.toInt() > 10) { uvIcon = "uvPurple";}
  drawBmpFromSdCard(uvIcon + ".bmp", 125, 227);
  String windSpeed = metOffice.getThirdWindSpeed();
  String windGust = metOffice.getThirdWindGust();
  ui.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setFont(&LiberationSerifBold_14);
  ui.drawString(145, 287, windSpeed + "/" + windGust);
}

void drawFourthForecast() {
  String forecastTime = metOffice.getFourthForecastTime();
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  tft.setFont(&LiberationSerifBold_14);
  ui.drawString(203, 75, forecastTime);
  String weatherType = metOffice.getFourthWeatherType();
  String weatherIcon = metOffice.getWeatherIconName(weatherType);
  drawBmpFromSdCard(weatherIcon + ".bmp", 182, 80);
  tft.setFont(&LiberationSerifBold_36);
  ui.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  ui.setTextAlignment(CENTER);
  String temperature = metOffice.getFourthTemperature();
  ui.drawString(201, 157, temperature);
  ui.setTextColor(ILI9341_ORANGE, ILI9341_BLACK);
  String feelsLikeTemp = metOffice.getFourthFeelsLikeTemp();
  ui.drawString(201, 196, feelsLikeTemp);
  ui.setTextColor(ILI9341_BLUE, ILI9341_BLACK);
  String precipProb = metOffice.getFourthPrecipProb();
  tft.setFont(&LiberationSerifBold_20);
  ui.drawString(201, 222, precipProb + "%");
  String maxUvIndex = metOffice.getFourthMaxUvIndex();
  String uvIcon = "";
  if (maxUvIndex.toInt() <= 2) { uvIcon = "uvGreen";}
  if (3 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 5) { uvIcon = "uvYellow";}
  if (6 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 7) { uvIcon = "uvOrange";}
  if (8 <= maxUvIndex.toInt() && maxUvIndex.toInt() <= 10) { uvIcon = "uvRed";}
  if (maxUvIndex.toInt() > 10) { uvIcon = "uvPurple";}
  drawBmpFromSdCard(uvIcon + ".bmp", 182, 227);
  String windSpeed = metOffice.getFourthWindSpeed();
  String windGust = metOffice.getFourthWindGust();
  ui.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  tft.setFont(&LiberationSerifBold_14);
  ui.drawString(201, 287, windSpeed + "/" + windGust);
}
