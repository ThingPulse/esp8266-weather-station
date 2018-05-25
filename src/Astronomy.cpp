/**The MIT License (MIT)

Copyright (c) 2018 by Daniel Eichhorn, ThingPulse

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

See more at https://thingpulse.com
*/
#include <time.h>
#include "Astronomy.h"

Astronomy::Astronomy() {

}

/**
* Convenience method to calculate moonphase by unix time stamp.
* See calculateMoonPhase(int year, int month, int day) for more details.
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
