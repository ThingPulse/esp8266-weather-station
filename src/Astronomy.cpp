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

#include "Astronomy.h"

/* pi/180 */
#define RPD 1.74532925199e-2

Astronomy::Astronomy() {

}

/**
 * Convenience method to calculate moon phase by unix time stamp. See calculateMoonPhase(int year, int month, int day)
 * for details.
 */
uint8_t Astronomy::calculateMoonPhase(time_t timestamp) {
  struct tm* timeInfo;
  timeInfo = localtime(&timestamp);
  return calculateMoonPhase(timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday);
}
/**
 * Calculates the moon phase for a given date, accurate to 1 segment. The result is in the range 0..7.
 * 0 => new moon
 * 4 => full moon
 *
 * Source: https://www.voidware.com/moon_phase.htm
 */
uint8_t Astronomy::calculateMoonPhase(uint16_t year, uint8_t month, uint8_t day) {

  // This floating point moon phase algorithm works by simply dividing the lunar month of 29.53 days into the number
  // of days elapsed since a known new moon. So, it’s highly simplified, but should be good enough to get quarter
  // phases.

  uint32_t yearAsDays;
  uint16_t monthAsDays;
  double daysSinceReferenceNewMoon;
  double moonCycles;
  uint16_t completedMoonCycles;
  double moonAge;
  uint8_t phase;

  // This adjustment ultimately comes from the calculation to turn y/m/d into a whole number of days offset. It’s a
  // modified version of the Julian Day number, but here it’s been simplified to work only between years 2000/1/1 and
  // 2099/12/31.
  if (month < 3) {
    year--;
    month += 12;
  }
  month++;

  yearAsDays = 365.25 * year;                                               // 365.25 -> mean length of a calendar year
  monthAsDays = 30.6 * month;                                               // 30.6 -> mean length of a month
  daysSinceReferenceNewMoon = yearAsDays + monthAsDays + day - 694039.09;   // number of days since known new moon on 1900-01-01, 694039.09 -> days elapsed since zero
  moonCycles = daysSinceReferenceNewMoon / 29.53;                           // 29.53 -> long-term average moon cycle duration in days
  completedMoonCycles = moonCycles;                                         // "casting" to int to get *completed* moon cycles i.e. only integer part
  moonAge = moonCycles - completedMoonCycles;                               // subtract integer part to leave fractional part which represents the current moon age
  phase = moonAge * 8 + 0.5;                                                // scale fraction from 0-8 and round by adding 0.5
  phase = phase & 7;                                                        // 0 and 8 are the same so turn 8 into 0
  return phase;
}

/**
 * Convenience method to calculate moon phase and illumination by unix time stamp. See
 * calculateMoonData(int year, int month, int day) for details.
 */
Astronomy::MoonData Astronomy::calculateMoonData(time_t timestamp) {
  struct tm* timeInfo;
  timeInfo = localtime(&timestamp);
  return calculateMoonData(timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday);
}
/**
 * Calculates the moon phase and illumination for a given date, accurate to 1 segment.
 * The result is in the range 0..7 for the phase.
 * 0 => new moon
 * 4 => full moon
 * Illumination ranges from 0.0 (new moon) to 1.0 (full moon).
 *
 * Source: Hugh from https://www.voidware.com
 */
Astronomy::MoonData Astronomy::calculateMoonData(uint16_t year, uint8_t month, uint8_t day) {

  MoonData moonData;

  // from Gregorian year, month, day, calculate the Julian Day number
  uint8_t c;
  uint32_t jd = 0;
  if (month < 3) {
    --year;
    month += 10;
  } else month -= 2;

  c = year / 100;
  jd += 30.59 * month;
  jd += 365.25 * year;
  jd += day;
  jd += c / 4 - c;

  // adjust to Julian centuries from noon, the day specified.
  double t = (jd - 730455.5) / 36525;

  // following calculation from Astronomical Algorithms, Jean Meeus
  // D, M, MM from (47.2, 47.3, 47.3 page 338)

  // mean elongation of the moon
  double D = 297.8501921 + t * (445267.1114034 +
                                t * (-0.0018819 + t * (1.0 / 545868 - t / 113065000)));

  // suns mean anomaly
  double M = 357.5291092 + t * (35999.0502909 + t * (-0.0001536 + t / 24490000));

  // moons mean anomaly
  double MM = 134.9633964 +
              t * (477198.8675055 + t * (0.0087414 + t * (1.0 / 69699 - t / 14712000)));

  // (48.4 p346)
  double i = 180 - D
             - 6.289 * sin(MM * RPD)
             + 2.100 * sin(M * RPD)
             - 1.274 * sin((2 * D - MM) * RPD)
             - 0.658 * sin(2 * D * RPD)
             - 0.214 * sin(2 * MM * RPD)
             - 0.110 * sin(D * RPD);

  if (i < 0) i = -i;
  if (i >= 360) i -= floor(i / 360) * 360;

  // (48.1 p 345)
  // this is the proportion illuminated calculated from `i`, the phase angle
  double k = (1 + cos(i * RPD)) / 2;

  // but for the `phase` don't use the phase angle
  // instead just consider the 0-360 cycle to get equal parts per phase
  uint8_t ki = i / 22.5;
  if (++ki == 16) ki = 0;
  ki = (ki / 2 + 4) & 7;

  moonData.phase = ki;
  moonData.illumination = k;

  return moonData;
}
