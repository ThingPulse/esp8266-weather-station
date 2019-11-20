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

*** CREDIT ***
This implementation is based on a Java class by T. Alonso Albi from OAN (Spain)
 published at http://conga.oan.es/~alonso/doku.php?id=blog:sun_moon_position
*/

#include "SunMoonCalc.h"

/** Radians to hours. */
const double RAD_TO_HOUR = 180.0 / (15.0 * PI);

/** Radians to days. */
const double RAD_TO_DAY = RAD_TO_HOUR / 24.0;

/** Astronomical Unit in km. As defined by JPL. */
const double AU = 149597870.691;

/** Earth equatorial radius in km. IERS 2003 Conventions. */
const double EARTH_RADIUS = 6378.1366;

/** The inverse of two times Pi. */
const double TWO_PI_INVERSE = 1.0 / TWO_PI;

/** Four times Pi. */
const double FOUR_PI = 2.0 * TWO_PI;

/** Pi divided by two. */
const double PI_OVER_TWO = PI / 2.0;

/** Length of a sidereal day in days according to IERS Conventions. */
const double SIDEREAL_DAY_LENGTH = 1.00273781191135448;

/** Julian century conversion constant = 100 * days per year. */
const double JULIAN_DAYS_PER_CENTURY = 36525.0;

/** Seconds in one day. */
const double SECONDS_PER_DAY = 86400;

/** Our default epoch. The Julian Day which represents noon on 2000-01-01. */
const double J2000 = 2451545.0;

/** Lunar cycle length in days*/
const double LUNAR_CYCLE_DAYS = 29.530588853;


/*****************************************************************************
 *
 * Function timegm is like mktime but for UTC rather than local time.  As such
 *          it is the inverse of gmtime. It creates a time_t epoch instant
 *          from a tm struct. Obviously the values in the struct are expected
 *          to be in UTC. Unfortunately it is missing in ESP8266 Arduino-core,
 *          see https://github.com/esp8266/Arduino/issues/4820
 *
 * Source: http://www.thebackshed.com/forum/forum_posts.asp?TID=10023&PN=12
 *
 */
#define EPOCH_YR 1970            /* EPOCH = Jan 1 1970 00:00:00 */

const int _ytab[2][12] = {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

/* Number of days per month (except for February in leap years). */
static const int monoff[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

static int is_leap_year(int year) {
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

static int leap_days(int y1, int y2) {
  --y1;
  --y2;
  return (y2 / 4 - y1 / 4) - (y2 / 100 - y1 / 100) + (y2 / 400 - y1 / 400);
}

time_t timegm(const struct tm *tm) {
  int year;
  time_t days;
  time_t hours;
  time_t minutes;
  time_t seconds;

  year = 1900 + tm->tm_year;
  days = 365 * (year - EPOCH_YR) + leap_days(EPOCH_YR, year);
  days += monoff[tm->tm_mon];

  if (tm->tm_mon > 1 && is_leap_year(year))
    ++days;
  days += tm->tm_mday - 1;

  hours = days * 24 + tm->tm_hour;
  minutes = hours * 60 + tm->tm_min;
  seconds = minutes * 60 + tm->tm_sec;

  return seconds;
}
/** END timegm ***************************************************************/


/**
 * Convenience constructor that accepts an epoch instant rather than individual calendar fields.
 *
 * @param timestamp epoch instant
 * @param lat latitude for the observer in degrees
 * @param lon longitude for the observer in degrees
 */
SunMoonCalc::SunMoonCalc(const time_t timestamp, const double lat, const double lon) {
  struct tm* tm = gmtime(&timestamp);
  *this = SunMoonCalc(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, lat, lon);
}

/**
 * Main constructor for Sun/Moon calculations. Time should be given in Universal Time (UTC), observer angles in degrees. 
 *
 * @param year supports negative values
 * @param month the month in the 1-12 range
 * @param day day of the month
 * @param h the hour in the 0-23 range (i.e. 24-hour clock)
 * @param minute
 * @param second
 * @param lat latitude for the observer in degrees
 * @param lon longitude for the observer in degrees
 */
SunMoonCalc::SunMoonCalc(const int16_t year, const uint8_t month, const uint8_t day, const uint8_t hour,
                         const uint8_t minute, const uint8_t second, const double lat, const double lon) {

  setInternalTime(year, month, day, hour, minute, second);
  this->lat = radians(lat);
  this->lon = radians(lon);
}

SunMoonCalc::Result SunMoonCalc::calculateSunAndMoonData(){
  double jd = this->jd_UT;

  Result result;
  Sun sun;
  Moon moon;

  // First the Sun
  double *out = doCalc(getSunPosition());
  sun.azimuth = out[0];
  sun.elevation = out[1];
  sun.riseJd = out[2];
  sun.setJd = out[3];
  sun.transitJd = out[4];
  sun.transitElevation = out[5];
  sun.distance = out[8];
  double sa = this->sanomaly, sl = this->slongitude;

  double sunRA = out[6], sunDec = out[7], lst = out[9];

  int iterations = 3; // Number of iterations to get accurate rise/set/transit times
  sun.riseJd = obtainAccurateRiseSetTransit(sun.riseJd, 2, iterations, true);
  sun.setJd = obtainAccurateRiseSetTransit(sun.setJd, 3, iterations, true);
  sun.transitJd = obtainAccurateRiseSetTransit(sun.transitJd, 4, iterations, true);
  if (sun.transitJd == -1) {
    sun.transitElevation = 0;
  } else {
    // Update Sun's maximum elevation
    setUTDate(sun.transitJd);
    delete[] out;
    out = doCalc(getSunPosition());
    sun.transitElevation = out[5];
  }

  // Now Moon
  setUTDate(jd);
  this->sanomaly = sa;
  this->slongitude = sl;

  const PositionalData moonPosition = getMoonPosition();
  double moonLat = moonPosition.latitude, moonLon = moonPosition.longitude;
  delete[] out;
  out = doCalc(moonPosition);

  moon.azimuth = out[0];
  moon.elevation = out[1];
  moon.riseJd = out[2];
  moon.setJd = out[3];
  moon.transitJd = out[4];
  moon.transitElevation = out[5];
  moon.distance = out[8];

  moon.illumination = (1 - cos(this->moonAge / LUNAR_CYCLE_DAYS * 2 * PI)) / 2;
  double moonRA = out[6], moonDec = out[7];

  double ma = this->moonAge;

  iterations = 5; // Number of iterations to get accurate rise/set/transit times
  moon.riseJd = obtainAccurateRiseSetTransit(moon.riseJd, 2, iterations, false);
  moon.setJd = obtainAccurateRiseSetTransit(moon.setJd, 3, iterations, false);
  moon.transitJd = obtainAccurateRiseSetTransit(moon.transitJd, 4, iterations, false);
  if (moon.transitJd == -1) {
    moon.transitElevation = 0;
  } else {
    // Update Moon's maximum elevation
    setUTDate(moon.transitJd);
    getSunPosition();
    delete[] out;
    out = doCalc(getMoonPosition());
    moon.transitElevation = out[5];
  }
  setUTDate(jd);
  this->sanomaly = sa;
  this->slongitude = sl;
  this->moonAge = ma;
  
  delete[] out;
  out = getMoonDiskOrientationAngles(lst, sunRA, sunDec, radians(moonLon),radians(moonLat), moonRA, moonDec);
  moon.axisPositionAngle = out[2];
  moon.brightLimbAngle = out[3];
  moon.paralacticAngle = out[4];
  delete[] out;
  moon.age = ma;
  moon.phase = calculateMoonPhase(moon.age);

  result.sun = sun;
  result.moon = moon;

  return translateToHumanReadable(result);
}

SunMoonCalc::PositionalData SunMoonCalc::getSunPosition() {
  PositionalData position;

  // SUN PARAMETERS (Formulae from "Calendrical Calculations")
  double lon = (280.46645 + 36000.76983 * this->t + .0003032 * this->t * this->t);
  double anom = (357.5291 + 35999.0503 * this->t - .0001559 * this->t * this->t -
                 4.8E-07 * this->t * this->t * this->t);
  this->sanomaly = anom * DEG_TO_RAD;
  double c = (1.9146 - .004817 * this->t - .000014 * this->t * this->t) * sin(this->sanomaly);
  c = c + (.019993 - .000101 * this->t) * sin(2 * this->sanomaly);
  c = c + .00029 * sin(3.0 * this->sanomaly); // Correction to the mean ecliptic longitude

  // Now, let calculate nutation and aberration
  double M1 = (124.90 - 1934.134 * this->t + 0.002063 * this->t * this->t) * DEG_TO_RAD;
  double M2 = (201.11 + 72001.5377 * this->t + 0.00057 * this->t * this->t) * DEG_TO_RAD;
  double d = -.00569 - .0047785 * sin(M1) - .0003667 * sin(M2);

  this->slongitude = lon + c + d; // apparent longitude (error<0.003 deg)
  double slatitude = 0; // Sun's ecliptic latitude is always negligible
  double ecc = .016708617 - 4.2037E-05 * this->t - 1.236E-07 * this->t * this->t; // Eccentricity
  double v = this->sanomaly + c * DEG_TO_RAD; // true anomaly
  double sdistance = 1.000001018 * (1.0 - ecc * ecc) / (1.0 + ecc * cos(v)); // In UA

  position.longitude = this->slongitude;
  position.latitude = slatitude;
  position.distance = sdistance;
  position.angularRadius = atan(696000 / (AU * sdistance));

  return position;
}

SunMoonCalc::PositionalData SunMoonCalc::getMoonPosition() {
  PositionalData position;

  // MOON PARAMETERS (Formulae from "Calendrical Calculations")
  double phase = normalizeRadians(
          (297.8502042 + 445267.1115168 * this->t - 0.00163 * this->t * this->t + this->t * this->t * this->t / 538841 -
           this->t * this->t * this->t * this->t / 65194000) * DEG_TO_RAD);

  // Anomalistic phase
  double anomaly = (134.9634114 + 477198.8676313 * this->t + .008997 * this->t * this->t +
                    this->t * this->t * this->t / 69699 - this->t * this->t * this->t * this->t / 14712000);
  anomaly = anomaly * DEG_TO_RAD;

  // Degrees from ascending node
  double node = (93.2720993 + 483202.0175273 * this->t - 0.0034029 * this->t * this->t -
                 this->t * this->t * this->t / 3526000 + this->t * this->t * this->t * this->t / 863310000);
  node = node * DEG_TO_RAD;

  double E = 1.0 - (.002495 + 7.52E-06 * (this->t + 1.0)) * (this->t + 1.0);

  // Now longitude, with the three main correcting terms of evection,
  // variation, and equation of year, plus other terms (error<0.01 deg)
  // P. Duffet's MOON program taken as reference
  double l = (218.31664563 + 481267.8811958 * this->t - .00146639 * this->t * this->t +
              this->t * this->t * this->t / 540135.03 - this->t * this->t * this->t * this->t / 65193770.4);
  l += 6.28875 * sin(anomaly) + 1.274018 * sin(2 * phase - anomaly) + .658309 * sin(2 * phase);
  l += 0.213616 * sin(2 * anomaly) - E * .185596 * sin(this->sanomaly) - 0.114336 * sin(2 * node);
  l += .058793 * sin(2 * phase - 2 * anomaly) + .057212 * E * sin(2 * phase - anomaly - this->sanomaly) +
       .05332 * sin(2 * phase + anomaly);
  l += .045874 * E * sin(2 * phase - this->sanomaly) + .041024 * E * sin(anomaly - this->sanomaly) -
       .034718 * sin(phase) -
       E * .030465 * sin(this->sanomaly + anomaly);
  l += .015326 * sin(2 * (phase - node)) - .012528 * sin(2 * node + anomaly) - .01098 * sin(2 * node - anomaly) +
       .010674 * sin(4 * phase - anomaly);
  l += .010034 * sin(3 * anomaly) + .008548 * sin(4 * phase - 2 * anomaly);
  l += -E * .00791 * sin(this->sanomaly - anomaly + 2 * phase) - E * .006783 * sin(2 * phase + this->sanomaly) +
       .005162 * sin(anomaly - phase) + E * .005 * sin(this->sanomaly + phase);
  l += .003862 * sin(4 * phase) + E * .004049 * sin(anomaly - this->sanomaly + 2 * phase) +
       .003996 * sin(2 * (anomaly + phase)) + .003665 * sin(2 * phase - 3 * anomaly);
  l += E * 2.695E-3 * sin(2 * anomaly - this->sanomaly) + 2.602E-3 * sin(anomaly - 2 * (node + phase));
  l += E * 2.396E-3 * sin(2 * (phase - anomaly) - this->sanomaly) - 2.349E-3 * sin(anomaly + phase);
  l += E * E * 2.249E-3 * sin(2 * (phase - this->sanomaly)) - E * 2.125E-3 * sin(2 * anomaly + this->sanomaly);
  l += -E * E * 2.079E-3 * sin(2 * this->sanomaly) + E * E * 2.059E-3 * sin(2 * (phase - this->sanomaly) - anomaly);
  l += -1.773E-3 * sin(anomaly + 2 * (phase - node)) - 1.595E-3 * sin(2 * (node + phase));
  l += E * 1.22E-3 * sin(4 * phase - this->sanomaly - anomaly) - 1.11E-3 * sin(2 * (anomaly + node));
  double longitude = l;

  // Let's add nutation here also
  double M1 = (124.90 - 1934.134 * this->t + 0.002063 * this->t * this->t) * DEG_TO_RAD;
  double M2 = (201.11 + 72001.5377 * this->t + 0.00057 * this->t * this->t) * DEG_TO_RAD;
  double d = -.0047785 * sin(M1) - .0003667 * sin(M2);
  longitude += d;

  // Get accurate Moon age
  double Psin = LUNAR_CYCLE_DAYS;
  this->moonAge = normalizeRadians((longitude - this->slongitude) * DEG_TO_RAD) * Psin / TWO_PI;

  // Now Moon parallax
  double parallax = .950724 + .051818 * cos(anomaly) + .009531 * cos(2 * phase - anomaly);
  parallax += .007843 * cos(2 * phase) + .002824 * cos(2 * anomaly);
  parallax += 0.000857 * cos(2 * phase + anomaly) + E * .000533 * cos(2 * phase - this->sanomaly);
  parallax += E * .000401 * cos(2 * phase - anomaly - this->sanomaly) + E * .00032 * cos(anomaly - this->sanomaly) -
              .000271 * cos(phase);
  parallax += -E * .000264 * cos(this->sanomaly + anomaly) - .000198 * cos(2 * node - anomaly);
  parallax += 1.73E-4 * cos(3 * anomaly) + 1.67E-4 * cos(4 * phase - anomaly);

  // So Moon distance in Earth radii is, more or less,
  double distance = 1.0 / sin(parallax * DEG_TO_RAD);

  // Ecliptic latitude with nodal phase (error<0.01 deg)
  l = 5.128189 * sin(node) + 0.280606 * sin(node + anomaly) + 0.277693 * sin(anomaly - node);
  l += .173238 * sin(2 * phase - node) + .055413 * sin(2 * phase + node - anomaly);
  l += .046272 * sin(2 * phase - node - anomaly) + .032573 * sin(2 * phase + node);
  l += .017198 * sin(2 * anomaly + node) + .009267 * sin(2 * phase + anomaly - node);
  l += .008823 * sin(2 * anomaly - node) + E * .008247 * sin(2 * phase - this->sanomaly - node) +
       .004323 * sin(2 * (phase - anomaly) - node);
  l += .0042 * sin(2 * phase + node + anomaly) + E * .003372 * sin(node - this->sanomaly - 2 * phase);
  l += E * 2.472E-3 * sin(2 * phase + node - this->sanomaly - anomaly);
  l += E * 2.222E-3 * sin(2 * phase + node - this->sanomaly);
  l += E * 2.072E-3 * sin(2 * phase - node - this->sanomaly - anomaly);
  double latitude = l;

  position.longitude = longitude;
  position.latitude = latitude;
  position.distance = distance * EARTH_RADIUS / AU;
  position.angularRadius = atan(1737.4 / (distance * EARTH_RADIUS));

  return position;
}

/**
 * Returns a double array with 10 elements:
 * - azimuth
 * - elevation
 * - rise
 * - set
 * - transit
 * - transit elevation
 * - ra
 * - dec
 * - distance
 * - lst
 */
double *SunMoonCalc::doCalc(PositionalData position) {
  double* arr = new double[10];

  // Ecliptic to equatorial coordinates
  double t2 = this->t / 100.0;
  double tmp = t2 * (27.87 + t2 * (5.79 + t2 * 2.45));
  tmp = t2 * (-249.67 + t2 * (-39.05 + t2 * (7.12 + tmp)));
  tmp = t2 * (-1.55 + t2 * (1999.25 + t2 * (-51.38 + tmp)));
  tmp = (t2 * (-4680.93 + tmp)) / 3600.0;
  double angle = (23.4392911111111 + tmp) * DEG_TO_RAD; // obliquity

  // Add nutation in obliquity
  double M1 = (124.90 - 1934.134 * this->t + 0.002063 * this->t * this->t) * DEG_TO_RAD;
  double M2 = (201.11 + 72001.5377 * this->t + 0.00057 * this->t * this->t) * DEG_TO_RAD;
  double d = .002558 * cos(M1) - .00015339 * cos(M2);
  angle += d * DEG_TO_RAD;

  position.longitude *= DEG_TO_RAD;
  position.latitude *= DEG_TO_RAD;
  double cl = cos(position.latitude);
  double x = position.distance * cos(position.longitude) * cl;
  double y = position.distance * sin(position.longitude) * cl;
  double z = position.distance * sin(position.latitude);
  tmp = y * cos(angle) - z * sin(angle);
  z = y * sin(angle) + z * cos(angle);
  y = tmp;

  // Obtain local apparent sidereal time
  double jd0 = floor(this->jd_UT - 0.5) + 0.5;
  double T0 = (jd0 - J2000) / JULIAN_DAYS_PER_CENTURY;
  double secs = (this->jd_UT - jd0) * SECONDS_PER_DAY;
  double gmst = (((((-6.2e-6 * T0) + 9.3104e-2) * T0) + 8640184.812866) * T0) + 24110.54841;
  double msday =
          1.0 + (((((-1.86e-5 * T0) + 0.186208) * T0) + 8640184.812866) / (SECONDS_PER_DAY * JULIAN_DAYS_PER_CENTURY));
  gmst = (gmst + msday * secs) * (15.0 / 3600.0) * DEG_TO_RAD;
  double lst = gmst + this->lon;

  // Obtain topocentric rectangular coordinates
  // Set radiusAU = 0 for geocentric calculations
  // (rise/set/transit will have no sense in this case)
  double radiusAU = EARTH_RADIUS / AU;
  double correction[3] = {
          radiusAU * cos(this->lat) * cos(lst),
          radiusAU * cos(this->lat) * sin(lst),
          radiusAU * sin(this->lat)};
  double xtopo = x - correction[0];
  double ytopo = y - correction[1];
  double ztopo = z - correction[2];

  // Obtain topocentric equatorial coordinates
  double ra = 0.0;
  double dec = PI_OVER_TWO;
  if (ztopo < 0.0) {
    dec = -dec;
  }
  if (ytopo != 0.0 || xtopo != 0.0) {
    ra = atan2(ytopo, xtopo);
    dec = atan2(ztopo / sqrt(xtopo * xtopo + ytopo * ytopo), 1.0);
  }
  double dist = sqrt(xtopo * xtopo + ytopo * ytopo + ztopo * ztopo);

  // Hour angle
  double angh = lst - ra;

  // Obtain azimuth and geometric alt
  double sinlat = sin(this->lat);
  double coslat = cos(this->lat);
  double sindec = sin(dec), cosdec = cos(dec);
  double h = sinlat * sindec + coslat * cosdec * cos(angh);
  double alt = asin(h);
  double azy = sin(angh);
  double azx = cos(angh) * sinlat - sindec * coslat / cosdec;
  double azi = PI + atan2(azy, azx); // 0 = north

  // Get apparent elevation
  if (alt > -3 * DEG_TO_RAD) {
    double r = 0.016667 * DEG_TO_RAD * fabs(tan(PI_OVER_TWO - (alt * RAD_TO_DEG +  7.31 / (alt * RAD_TO_DEG + 4.4)) * DEG_TO_RAD));
    double refr = r * ( 0.28 * 1010 / (10 + 273.0)); // Assuming pressure of 1010 mb and T = 10 C
    alt = fmin(alt + refr, PI_OVER_TWO); // This is not accurate, but acceptable
  }

  tmp = calculateTwilightAdjustment(position);

  // Compute cosine of hour angle
  tmp = (sin(tmp) - sin(this->lat) * sin(dec)) / (cos(this->lat) * cos(dec));
  double celestialHoursToEarthTime = RAD_TO_DAY / SIDEREAL_DAY_LENGTH;

  // Make calculations for the meridian
  double transit_time1 = celestialHoursToEarthTime * normalizeRadians(ra - lst);
  double transit_time2 = celestialHoursToEarthTime * (normalizeRadians(ra - lst) - TWO_PI);
  double transit_alt = asin(sin(dec) * sin(this->lat) + cos(dec) * cos(this->lat));
  if (transit_alt > -3 * DEG_TO_RAD) {
    double r = 0.016667 * DEG_TO_RAD * fabs(tan(PI_OVER_TWO - (transit_alt * RAD_TO_DEG +  7.31 / (transit_alt * RAD_TO_DEG + 4.4)) * DEG_TO_RAD));
    double refr = r * ( 0.28 * 1010 / (10 + 273.0)); // Assuming pressure of 1010 mb and T = 10 C
    transit_alt = fmin(transit_alt + refr, PI_OVER_TWO); // This is not accurate, but acceptable
  }

  // Obtain the current event in time
  double transit_time = transit_time1;
  double jdToday = floor(this->jd_UT - 0.5) + 0.5;
  double transitToday2 = floor(this->jd_UT + transit_time2 - 0.5) + 0.5;
  // Obtain the transit time. Preference should be given to the closest event
  // in time to the current calculation time
  if (jdToday == transitToday2 && fabs(transit_time2) < fabs(transit_time1)) transit_time = transit_time2;
  double transit = this->jd_UT + transit_time;

  // Make calculations for rise and set
  double rise = -1, set = -1;
  if (fabs(tmp) <= 1.0) {
    double ang_hor = fabs(acos(tmp));
    double rise_time1 = celestialHoursToEarthTime * normalizeRadians(ra - ang_hor - lst);
    double set_time1 = celestialHoursToEarthTime * normalizeRadians(ra + ang_hor - lst);
    double rise_time2 = celestialHoursToEarthTime * (normalizeRadians(ra - ang_hor - lst) - TWO_PI);
    double set_time2 = celestialHoursToEarthTime * (normalizeRadians(ra + ang_hor - lst) - TWO_PI);

    // Obtain the current events in time. Preference should be given to the closest event
    // in time to the current calculation time (so that iteration in other method will converge)
    double rise_time = rise_time1;
    double riseToday2 = floor(this->jd_UT + rise_time2 - 0.5) + 0.5;
    if (jdToday == riseToday2 && fabs(rise_time2) < fabs(rise_time1)) rise_time = rise_time2;

    double set_time = set_time1;
    double setToday2 = floor(this->jd_UT + set_time2 - 0.5) + 0.5;
    if (jdToday == setToday2 && fabs(set_time2) < fabs(set_time1)) set_time = set_time2;
    rise = this->jd_UT + rise_time;
    set = this->jd_UT + set_time;
  }

  arr[0] = azi;
  arr[1] = alt;
  arr[2] = rise;
  arr[3] = set;
  arr[4] = transit;
  arr[5] = transit_alt;
  arr[6] = ra;
  arr[7] = dec;
  arr[8] = dist;
  arr[9] = lst;

  return arr;
}

/**
 * Initializes the internal time variables t and jd_UT from the UTC timestamp given.
 */
void SunMoonCalc::setInternalTime(const int16_t year, const uint8_t month, const uint8_t day, const uint8_t hour,
                                  const uint8_t minute, const uint8_t second) {
    
  double jd = toJulian(year, month, day, hour, minute, second);

  this->TTminusUT = 0;
  if (year > -600 && year < 2200) {
    double x = year + (month - 1 + day / 30.0) / 12.0;
    double x2 = x * x, x3 = x2 * x, x4 = x3 * x;
    if (year < 1600) {
      this->TTminusUT =
              10535.328003326353 - 9.995238627481024 * x + 0.003067307630020489 * x2 - 7.76340698361363E-6 * x3 +
              3.1331045394223196E-9 * x4 +
              8.225530854405553E-12 * x2 * x3 - 7.486164715632051E-15 * x4 * x2 + 1.9362461549678834E-18 * x4 * x3 -
              8.489224937827653E-23 * x4 * x4;
    } else {
      this->TTminusUT =
              -1027175.3477559977 + 2523.256625418965 * x - 1.885686849058459 * x2 + 5.869246227888417E-5 * x3 +
              3.3379295816475025E-7 * x4 +
              1.7758961671447929E-10 * x2 * x3 - 2.7889902806153024E-13 * x2 * x4 +
              1.0224295822336825E-16 * x3 * x4 - 1.2528102370680435E-20 * x4 * x4;
    }
  }
  setUTDate(jd);
}

void SunMoonCalc::setUTDate(const double jd) {
  this->jd_UT = jd;
  this->t = (jd + this->TTminusUT / SECONDS_PER_DAY - J2000) / JULIAN_DAYS_PER_CENTURY;
}

double SunMoonCalc::calculateTwilightAdjustment(PositionalData position) const {
  double adjustment = 0.0;
  switch (twilight) {
    case HORIZON_34arcmin:
      // Rise, set, transit times, taking into account Sun/Moon angular radius (position[3]).
      // The 34' factor is the standard refraction at horizon.
      // Removing angular radius will do calculations for the center of the disk instead
      // of the upper limb.
      adjustment = -(34.0 / 60.0) * DEG_TO_RAD - position.angularRadius;
      break;
    case TWILIGHT_CIVIL:
      adjustment = -6 * DEG_TO_RAD;
      break;
    case TWILIGHT_NAUTICAL:
      adjustment = -12 * DEG_TO_RAD;
      break;
    case TWILIGHT_ASTRONOMICAL:
      adjustment = -18 * DEG_TO_RAD;
      break;
  }
  return adjustment;
}

double SunMoonCalc::obtainAccurateRiseSetTransit(double riseSetJd, const int index, const int niter, const bool sun) {
  double step = -1;
  for (int i = 0; i < niter; i++) {
    if (riseSetJd == -1) return riseSetJd; // -1 means no rise/set from that location
    setUTDate(riseSetJd);
    double *out = nullptr;
    if (sun) {
      out = doCalc(getSunPosition());
    } else {
      getSunPosition();
      out = doCalc(getMoonPosition());
    }
    step = fabs(riseSetJd - out[index]);
    riseSetJd = out[index];
    delete[] out;
  }
  if (step > 1.0 / SECONDS_PER_DAY) return -1; // did not converge => without rise/set/transit in this date
  return riseSetJd;
}

/**
 * Returns double array with 5 elements:
 * - optical librations (lp)
 * - lunar coordinates of the centre of the disk (bp)
 * - position angle of axis (p)
 * - bright limb angle (bl)
 * - paralactic angle (par)}
 */
double *SunMoonCalc::getMoonDiskOrientationAngles(double lst, double sunRA, double sunDec, double moonLon,
                                                  double moonLat, double moonRA, double moonDec) {

  double* arr = new double[5];
  
  // Moon's argument of latitude
  double F = radians(93.2720993 + 483202.0175273 * this->t - 0.0034029 * this->t * this->t -
                       this->t * this->t * this->t / 3526000.0 + this->t * this->t * this->t * this->t / 863310000.0);
  // Moon's inclination
  double I = radians(1.54242);
  // Moon's mean ascending node longitude
  double omega = radians(125.0445550 - 1934.1361849 * this->t + 0.0020762 * this->t * this->t +
                           this->t * this->t * this->t / 467410.0 - this->t * this->t * this->t * this->t / 18999000.0);
  // Obliquity of ecliptic (approx, better formulae up)
  double eps = radians(23.43929);

  // Obtain optical librations lp and bp
  double W = moonLon - omega;
  double sinA = sin(W) * cos(moonLat) * cos(I) - sin(moonLat) * sin(I);
  double cosA = cos(W) * cos(moonLat);
  double A = atan2(sinA, cosA);
  double lp = normalizeRadians(A - F);
  double sinbp = - sin(W) * cos(moonLat) * sin(I) - sin(moonLat) * cos(I);
  double bp = asin(sinbp);

  // Obtain position angle of axis p
  double x = sin(I) * sin(omega);
  double y = sin(I) * cos(omega) * cos(eps) - cos(I) * sin(eps);
  double w = atan2(x, y);
  double sinp = sqrt(x*x + y*y) * cos(moonRA - w) / cos(bp);
  double p = asin(sinp);

  // Compute bright limb angle bl
  double bl = (PI + atan2(cos(sunDec) * sin(moonRA - sunRA),
                          cos(sunDec) * sin(moonDec) * cos(moonRA - sunRA)
                          - sin(sunDec) * cos(moonDec)));

  // Paralactic angle par
  y = sin(lst - moonRA);
  x = tan(this->lat) * cos(moonDec) - sin(moonDec) * cos(lst - moonRA);
  double par = x != 0 ? atan2(y, x) : (y / fabs(y)) * PI / 2;

  arr[0] = lp;
  arr[1] = bp;
  arr[2] = p;
  arr[3] = bl;
  arr[4] = par;

  return arr;
}

SunMoonCalc::MoonPhase SunMoonCalc::calculateMoonPhase(double lunarAge) const {
  MoonPhase moonPhase = SunMoonCalc::MoonPhase();
  if (lunarAge >= 0 && lunarAge <= LUNAR_CYCLE_DAYS
      && (lunarAge < 1 || lunarAge > LUNAR_CYCLE_DAYS - 1)) {
    moonPhase.index = 0;
    moonPhase.name = "New Moon";
  } else if (lunarAge >= 1 && lunarAge < 6.4) {
    moonPhase.index = 1;
    moonPhase.name = "Waxing Crescent";
  } else if (lunarAge >= 6.4 && lunarAge < 8.4) {
    moonPhase.index = 2;
    moonPhase.name = "First Quarter";
  } else if (lunarAge >= 8.4 && lunarAge < 13.8) {
    moonPhase.index = 3;
    moonPhase.name = "Waxing Gibbous";
  } else if (lunarAge >= 13.8 && lunarAge < 15.8) {
    moonPhase.index = 4;
    moonPhase.name = "Full Moon";
  } else if (lunarAge >= 15.8 && lunarAge < 21.1) {
    moonPhase.index = 5;
    moonPhase.name = "Waning Gibbous";
  } else if (lunarAge >= 21.1 && lunarAge < 23.1) {
    moonPhase.index = 6;
    moonPhase.name = "Last/Third Quarter";
  } else if (lunarAge >= 23.1 && lunarAge <= LUNAR_CYCLE_DAYS - 1) {
    moonPhase.index = 7;
    moonPhase.name = "Waning Crescent";
  }
  return moonPhase;
}

/**
 * Reduces an angle in radians to the range (0 - 2 Pi).
 */
double SunMoonCalc::normalizeRadians(double r) {
  if (r < 0 && r >= -TWO_PI) return r + TWO_PI;
  if (r >= TWO_PI && r < FOUR_PI) return r - TWO_PI;
  if (r >= 0 && r < TWO_PI) return r;

  r -= TWO_PI * floor(r * TWO_PI_INVERSE);
  if (r < 0.) r += TWO_PI;

  return r;
}

/**
 * Transforms a Julian day (rise/set/transit fields) to a common date in UTC.
 */
time_t SunMoonCalc::fromJulian(double julianDays) const {
  struct tm tm{};

  // The conversion formulas are from Meeus, chapter 7.
  double Z = floor(julianDays + 0.5);
  double F = julianDays + 0.5 - Z;
  double A = Z;
  if (Z >= 2299161) {
    auto a = (int) ((Z - 1867216.25) / 36524.25);
    A += 1 + a - a / 4;
  }
  double B = A + 1524;
  auto C = (int) ((B - 122.1) / 365.25);
  auto D = (int) (C * 365.25);
  auto E = (int) ((B - D) / 30.6001);

  double exactDay = F + B - D - (int) (30.6001 * E);
  auto day = (int) exactDay;
  int month = (E < 14) ? E - 1 : E - 13;
  int year = C - 4715;
  if (month > 2) year--;
  double h = ((exactDay - day) * SECONDS_PER_DAY) / 3600.0;

  auto hour = (int) h;
  double m = (h - hour) * 60.0;
  auto minute = (int) m;
  auto second = (int) ((m - minute) * 60.0);

  tm.tm_year = year - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  tm.tm_hour = hour;
  tm.tm_min = minute;
  tm.tm_sec = second;

  return timegm(&tm);
}

double SunMoonCalc::toJulian(const int16_t year, const uint8_t month, const uint8_t day, const uint8_t hour,
                             const uint8_t minute, const uint8_t second) const {
  // The conversion formulas are from Meeus, chapter 7.
  bool julian = false;
  if (year < 1582 || (year == 1582 && month <= 10) || (year == 1582 && month == 10 && day < 15)) julian = true;
  int D = day;
  int M = month;
  int Y = year;
  if (M < 3) {
    Y--;
    M += 12;
  }
  int A = Y / 100;
  int B = julian ? 0 : 2 - A + A / 4;

  double dayFraction = (hour + (minute + (second / 60.0)) / 60.0) / 24.0;
  double jd = dayFraction + (int) (365.25 * (Y + 4716)) + (int) (30.6001 * (M + 1)) + D + B - 1524.5;

  return jd;
}

/**
 * Converts the data in the struct to human readable units (km instead of astronomical units, degree instead of
 * radians, epoch instant instead of Julian Days).
 */
SunMoonCalc::Result SunMoonCalc::translateToHumanReadable(SunMoonCalc::Result result) const {
  result.sun.rise = fromJulian(result.sun.riseJd);
  result.sun.transit = fromJulian(result.sun.transitJd);
  result.sun.set = fromJulian(result.sun.setJd);
  result.sun.distance = result.sun.distance * AU;
  result.sun.azimuth = degrees(result.sun.azimuth);
  result.sun.elevation = degrees(result.sun.elevation);
  result.sun.transitElevation = degrees(result.sun.transitElevation);
  result.moon.distance = result.moon.distance * AU;
  result.moon.azimuth = degrees(result.moon.azimuth);
  result.moon.elevation = degrees(result.moon.elevation);
  result.moon.transitElevation = degrees(result.moon.transitElevation);
  result.moon.rise = fromJulian(result.moon.riseJd);
  result.moon.transit = fromJulian(result.moon.transitJd);
  result.moon.set = fromJulian(result.moon.setJd);
  return result;
}
